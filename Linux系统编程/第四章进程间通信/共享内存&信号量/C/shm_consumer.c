#include "shm_common.h"
#include <unistd.h>

int main(){
    key_t shmkey = ftok(KEY_PATH, SHM_PROJ);
    int shmid = shmget(shmkey, sizeof(SharedBuffer), 0666);
    SharedBuffer* buf = (SharedBuffer*) shmat(shmid, NULL, 0);

    key_t semkey = ftok(KEY_PATH, SEM_PROJ);
    int semid = semget(semkey, 3, 0666);

    printf("开始消费，共计有10份。\n");
    for(int i = 0; i < 10; i++){
        sem_p(semid, 1);
        sem_p(semid, 2);

        int item = buf->buffer[buf->out];
        buf->out = (buf->out + 1) % BUFFER_SIZE;
        buf->count--;
        printf("消费: %d， 缓冲区：%d/%d\n", i, buf->count, BUFFER_SIZE);

        sem_v(semid, 2);
        sem_v(semid, 0);
        usleep(200000);
    }
    printf("消费完成！\n");
    shmdt(buf);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    return 0;
}