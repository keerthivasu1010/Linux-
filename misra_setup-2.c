/*
 * misra_setup.c
 *
 * Host-side helper for the ADAS ECU Process Supervisor project.
 * It creates a compilation database and include-path file, then runs
 * Cppcheck + MISRA C 2012 analysis.
 *
 * IMPORTANT:
 * - This helper is NOT production ECU code.
 * - Do NOT include this file in the project's MISRA source scan.
 * - It expects to be run from the project root:
 *       ./misra_setup
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CPPCHECK "/root/cppcheck-main/cppcheck"
#define MISRA_ADDON "/root/cppcheck-main/addons/misra.py"

#define MAX_CMD_LEN 4096
#define MAX_PATH_LEN 1024

static int run_command(const char *command)
{
    int rc;

    printf("\n============================================================\n");
    printf("RUN: %s\n", command);
    printf("============================================================\n");

    rc = system(command);

    if (rc == -1)
    {
        fprintf(stderr, "system() failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if (WIFEXITED(rc) == 0)
    {
        fprintf(stderr, "Command did not exit normally.\n");
        return EXIT_FAILURE;
    }

    if (WEXITSTATUS(rc) != 0)
    {
        fprintf(stderr, "Command returned exit status %d.\n", WEXITSTATUS(rc));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int write_compile_database(const char *root)
{
    FILE *fp;
    const char *files[] = {
        "src/logger.c",
        "src/main.c",
        "src/service.c",
        "src/signal_handler.c",
        "src/supervisor.c",
        "services/camera_service.c",
        "services/lane_detection_service.c",
        "services/object_detection_service.c",
        "services/sensor_service.c",
        "services/service_common.c",
        "tests/unit/test_service.c",
        "tests/unit/test_syscalls.c"
    };
    const size_t file_count = sizeof(files) / sizeof(files[0]);
    size_t i;

    fp = fopen("compile_commands.json", "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Cannot create compile_commands.json: %s\n",
                strerror(errno));
        return EXIT_FAILURE;
    }

    if (fprintf(fp, "[\n") < 0)
    {
        (void)fclose(fp);
        return EXIT_FAILURE;
    }

    for (i = 0U; i < file_count; ++i)
    {
        if (fprintf(fp,
                    "  {\"directory\":\"%s\","
                    "\"command\":\"gcc -D_POSIX_C_SOURCE=200809L "
                    "-Wall -Wextra -Wpedantic -std=c11 -O2 "
                    "-I%s/include -I%s/services -c %s -o /tmp/cppcheck_%zu.o\","
                    "\"file\":\"%s/%s\"}%s\n",
                    root,
                    root,
                    root,
                    files[i],
                    i,
                    root,
                    files[i],
                    (i + 1U < file_count) ? "," : "") < 0)
        {
            (void)fclose(fp);
            return EXIT_FAILURE;
        }
    }

    if (fprintf(fp, "]\n") < 0)
    {
        (void)fclose(fp);
        return EXIT_FAILURE;
    }

    if (fclose(fp) != 0)
    {
        fprintf(stderr, "Cannot close compile_commands.json.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int write_include_paths(const char *root)
{
    FILE *fp;

    fp = fopen("cppcheck_includes.txt", "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Cannot create cppcheck_includes.txt: %s\n",
                strerror(errno));
        return EXIT_FAILURE;
    }

    if (fprintf(fp, "%s/include\n%s/services\n", root, root) < 0)
    {
        (void)fclose(fp);
        return EXIT_FAILURE;
    }

    if (fclose(fp) != 0)
    {
        fprintf(stderr, "Cannot close cppcheck_includes.txt.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int file_exists(const char *path)
{
    return access(path, F_OK) == 0;
}

int main(void)
{
    char cwd[MAX_PATH_LEN];
    char command[MAX_CMD_LEN];
    int rc;

    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        fprintf(stderr, "getcwd() failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    printf("ADAS ECU Supervisor - Cppcheck/MISRA setup\n");
    printf("Project root: %s\n", cwd);

    if (!file_exists("src") ||
        !file_exists("services") ||
        !file_exists("tests") ||
        !file_exists("include"))
    {
        fprintf(stderr,
                "ERROR: Run this program from the ADAS project root.\n");
        return EXIT_FAILURE;
    }

    if (!file_exists(CPPCHECK))
    {
        fprintf(stderr, "ERROR: Cppcheck not found at:\n%s\n", CPPCHECK);
        fprintf(stderr,
                "Edit CPPCHECK at the top of this file if your path differs.\n");
        return EXIT_FAILURE;
    }

    if (!file_exists(MISRA_ADDON))
    {
        fprintf(stderr, "ERROR: MISRA addon not found at:\n%s\n",
                MISRA_ADDON);
        fprintf(stderr,
                "Edit MISRA_ADDON at the top of this file if your path differs.\n");
        return EXIT_FAILURE;
    }

    rc = write_compile_database(cwd);
    if (rc != EXIT_SUCCESS)
    {
        return rc;
    }

    rc = write_include_paths(cwd);
    if (rc != EXIT_SUCCESS)
    {
        return rc;
    }

    printf("\nGenerated:\n");
    printf("  compile_commands.json\n");
    printf("  cppcheck_includes.txt\n");

    /*
     * First run normal Cppcheck. This separates ordinary Cppcheck
     * defects from MISRA addon/configuration messages.
     */
    (void)snprintf(
        command,
        sizeof(command),
        "%s --project=compile_commands.json "
        "--platform=unix64 --library=posix "
        "--includes-file=cppcheck_includes.txt "
        "--enable=all --inconclusive "
        "--cppcheck-build-dir=cppcheck_build "
        "--xml --xml-version=2 2> cppcheck_report.xml",
        CPPCHECK);

    rc = run_command(command);
    if (rc != EXIT_SUCCESS)
    {
        fprintf(stderr,
                "\nCppcheck reported findings. See cppcheck_report.xml.\n");
    }

    /*
     * Then run the MISRA C 2012 addon with the same project database.
     * --platform=unix64 is important for a Linux/POSIX target.
     */
    (void)snprintf(
        command,
        sizeof(command),
        "%s --project=compile_commands.json "
        "--platform=unix64 --library=posix "
        "--includes-file=cppcheck_includes.txt "
        "--addon=%s --enable=all --inconclusive "
        "--cppcheck-build-dir=cppcheck_build "
        "2> misra_report.txt",
        CPPCHECK,
        MISRA_ADDON);

    {
        const int misra_rc = run_command(command);

        printf("\n============================================================\n");
        printf("ANALYSIS COMPLETE\n");
        printf("============================================================\n");
        printf("Cppcheck report : cppcheck_report.xml\n");
        printf("MISRA report     : misra_report.txt\n");
        printf("Build directory  : cppcheck_build/\n");
        printf("\nIf you see [misra-config], it means the MISRA addon still\n");
        printf("cannot resolve some configuration/symbol information.\n");
        printf("That is a configuration diagnostic, not automatically a\n");
        printf("MISRA rule violation. Do not suppress it blindly.\n");

        /*
         * Return the MISRA command status because this helper is intended
         * to be usable in CI/GitHub workflows.
         */
        if (misra_rc != EXIT_SUCCESS)
        {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
