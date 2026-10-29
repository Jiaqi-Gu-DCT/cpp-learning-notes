#include "shm_common.h"
#include <unistd.h>

int main(){
    key_t shmkey = ftok(KEY_PATH, SHM_PROJ);
    int shmid = shmget(shmkey, sizeof(SharedBuffer), IPC_CREAT | 0666);
    SharedBuffer* buf = (SharedBuffer*) shmat(shmid, NULL, 0);
    buf->in = buf->out = buf->count = 0;

    key_t semkey = ftok(KEY_PATH, SEM_PROJ);
    // empty  full  mutex
    int semid = semget(semkey, 3, IPC_CREAT | 0666);
    union semun arg;
    arg.val = BUFFER_SIZE; semctl(semid, 0, SETVAL, arg); // empty = 5
    arg.val = 0; semctl(semid, 1, SETVAL, arg); // full = 0
    arg.val = 1; semctl(semid, 2, SETVAL, arg); // mutex = 1

    printf("开始生产，共计10个数据。\n");
    for(int i = 0; i < 10; i++){
        sem_p(semid, 0);
        sem_p(semid, 2);

        buf->buffer[buf->in] = i;
        buf->in = (buf->in + 1) % BUFFER_SIZE;
        buf->count++;

        printf("生产: %d， 缓冲区：%d/%d\n", i, buf->count, BUFFER_SIZE);
        sem_v(semid, 2);
        sem_v(semid, 1);

        usleep(200000);
    }
    printf("生产完成！\n");
    shmdt(buf);
    return 0;
}