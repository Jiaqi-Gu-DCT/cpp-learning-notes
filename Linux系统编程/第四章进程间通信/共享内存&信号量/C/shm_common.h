#ifndef SHM_COMMON_H
#define SHM_COMMON_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>

#define KEY_PATH "/tmp"
#define SHM_PROJ 'S'
#define SEM_PROJ 'T'
#define BUFFER_SIZE 5

// 环形缓冲区
typedef struct{
    int buffer[BUFFER_SIZE]; // 数据存储
    int in; // 写入位置
    int out; // 读取位置
    int count; // 当前缓冲区内数据量
}SharedBuffer;

// 信号量需要的联合体
union semun{
    int val;
    struct sem_ds* buf;
    unsigned short* array;
};

// P操作
void sem_p(int semid, int semnum){
    struct sembuf op = {semnum,-1, 0};
    semop(semid, &op, 1);
}

// V操作
void sem_v(int semid, int semnum){
    struct sembuf op = {semnum, 1, 0};
    semop(semid, &op, 1);
}

#endif