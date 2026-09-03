#include <stdio.h>
#include <stdlib.h>
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

    // Generate the same key used by sender
    key = ftok("progfile", 65);

    // Create or access the message queue
    msgid = msgget(key, 0666 | IPC_CREAT);

    if (msgid == -1)
    {
        perror("msgget");
        exit(1);
    }

    // Receive message of type 1
    if (msgrcv(msgid, &msg, sizeof(msg.msg_text), 1, 0) == -1)
    {
        perror("msgrcv");
        exit(1);
    }

    printf("Message received: %s\n", msg.msg_text);

    // Remove the message queue
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}
