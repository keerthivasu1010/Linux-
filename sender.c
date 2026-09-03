#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct message {
    long msg_type;
    char msg_text[100];
};

int main()
{
    key_t key;
    int msgid;
    struct message msg;

    key = ftok("progfile", 65);

    msgid = msgget(key, 0666 | IPC_CREAT);

    if (msgid == -1)
    {
        perror("msgget");
        exit(1);
    }

    msg.msg_type = 1;

    printf("Enter message: ");
    fgets(msg.msg_text, sizeof(msg.msg_text), stdin);

    if (msgsnd(msgid, &msg, strlen(msg.msg_text) + 1, 0) == -1)
    {
        perror("msgsnd");
        exit(1);
    }

    printf("Message sent successfully.\n");

    return 0;
}
