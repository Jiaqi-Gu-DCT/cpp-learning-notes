#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define PATH_NAME "/tmp"
#define PROJ_ID 'M'

struct msgbuf{
    long type;
    char text[256];
};

int main(){
    key_t key = ftok(PATH_NAME, PROJ_ID);
    int msgqid = msgget(key, IPC_CREAT | 0666);
    if(msgqid == -1){perror("msgget"); exit(EXIT_FAILURE);}

    struct msgbuf msg;
    // 1.普通消息
    msg.type = 1;
    strcpy(msg.text, "this is a normal message.");
    msgsnd(msgqid, &msg, sizeof(msg.text) + 1, 0);
    printf("sending type 1：%s\n", msg.text);

    // 2.紧急消息
    msg.type = 2;
    strcpy(msg.text, "SERVER OVERLOAD!");
    msgsnd(msgqid, &msg, sizeof(msg.text) + 1, 0);
    printf("sending type 2: %s\n", msg.text);

    // 3.第二条普通消息
    msg.type = 3;
    strcpy(msg.text, "this is a another message.");
    msgsnd(msgqid, &msg, sizeof(msg.text) + 1, 0);
    printf("sending type 3：%s\n", msg.text);

    printf("sending is done, the id of queue is %d.\n", msgqid);
    return 0;
}
