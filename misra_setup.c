/*
 * misra_setup.c
 * Host-side helper for ADAS ECU Supervisor MISRA-C analysis.
 *
 * Build:
 *   gcc -std=c11 -Wall -Wextra -Wpedantic misra_setup.c -o misra_setup
 *
 * Run from the project root:
 *   ./misra_setup
 *
 * Do NOT include this helper itself in the production MISRA scan.
 */

#include <stdio.h>
#include <stdlib.h>

static int run_command(const char *command)
{
    int result = system(command);

    if (result != 0)
    {
        (void)fprintf(stderr, "FAILED: %s\n", command);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main(void)
{
    const char *commands[] = {
        "/root/cppcheck-main/cppcheck --version",

        "cat > compile_commands.json <<'EOF'\n"
        "[\n"
        "  {\"directory\":\".\",\"command\":\"gcc -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -std=c11 -O2 -Iinclude -Iservices -c src/logger.c -o src/logger.o\",\"file\":\"src/logger.c\"},\n"
        "  {\"directory\":\".\",\"command\":\"gcc -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -std=c11 -O2 -Iinclude -Iservices -c src/main.c -o src/main.o\",\"file\":\"src/main.c\"},\n"
        "  {\"directory\":\".\",\"command\":\"gcc -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -std=c11 -O2 -Iinclude -Iservices -c src/service.c -o src/service.o\",\"file\":\"src/service.c\"},\n"
        "  {\"directory\":\".\",\"command\":\"gcc -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -std=c11 -O2 -Iinclude -Iservices -c src/signal_handler.c -o src/signal_handler.o\",\"file\":\"src/signal_handler.c\"},\n"
        "  {\"directory\":\".\",\"command\":\"gcc -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -std=c11 -O2 -Iinclude -Iservices -c src/supervisor.c -o src/supervisor.o\",\"file\":\"src/supervisor.c\"},\n"
        "  {\"directory\":\".\",\"command\":\"gcc -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -std=c11 -O2 -Iinclude -Iservices -c services/camera_service.c -o services/camera_service.o\",\"file\":\"services/camera_service.c\"},\n"
        "  {\"directory\":\".\",\"command\":\"gcc -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -std=c11 -O2 -Iinclude -Iservices -c services/lane_detection_service.c -o services/lane_detection_service.o\",\"file\":\"services/lane_detection_service.c\"},\n"
        "  {\"directory\":\".\",\"command\":\"gcc -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -std=c11 -O2 -Iinclude -Iservices -c services/object_detection_service.c -o services/object_detection_service.o\",\"file\":\"services/object_detection_service.c\"},\n"
        "  {\"directory\":\".\",\"command\":\"gcc -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -std=c11 -O2 -Iinclude -Iservices -c services/sensor_service.c -o services/sensor_service.o\",\"file\":\"services/sensor_service.c\"},\n"
        "  {\"directory\":\".\",\"command\":\"gcc -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -std=c11 -O2 -Iinclude -Iservices -c services/service_common.c -o services/service_common.o\",\"file\":\"services/service_common.c\"},\n"
        "  {\"directory\":\".\",\"command\":\"gcc -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -std=c11 -O2 -Iinclude -Iservices -c tests/unit/test_service.c -o tests/unit/test_service.o\",\"file\":\"tests/unit/test_service.c\"},\n"
        "  {\"directory\":\".\",\"command\":\"gcc -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -std=c11 -O2 -Iinclude -Iservices -c tests/unit/test_syscalls.c -o tests/unit/test_syscalls.o\",\"file\":\"tests/unit/test_syscalls.c\"}\n"
        "]\n"
        "EOF",

        "test -s compile_commands.json",

        "/root/cppcheck-main/cppcheck --project=compile_commands.json --addon=/root/cppcheck-main/addons/misra.py --library=posix"
    };

    size_t i;
    const size_t count = sizeof(commands) / sizeof(commands[0]);

    for (i = 0U; i < count; ++i)
    {
        if (run_command(commands[i]) != EXIT_SUCCESS)
        {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
