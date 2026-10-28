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

    // 先接收紧急消息
    printf("first, receiving emergency.\n");
    while(msgrcv(msgqid, &msg, sizeof(msg.text), 2, IPC_NOWAIT) != -1){
        printf("received type 2: %s\n", msg.text);
    }

    // 再接收所有普通消息
    printf("then, receiving normal.\n");
    while(msgrcv(msgqid, &msg, sizeof(msg.text), 1, IPC_NOWAIT) != -1){
        printf("received type 1: %s\n", msg.text);
    }

    // 删除消息队列
    printf("now, deleting the message queue.\n");
    msgctl(msgqid, IPC_RMID, NULL);
    return 0;
}