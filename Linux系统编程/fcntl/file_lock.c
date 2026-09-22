// 掌握最经典的工程应用：用文件记录锁防止程序重复启动。直观感受建议性锁的效果，理解写锁的独占性。
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#define LOCK_FILE "lock/single.txt"

int try_get_lock(int fd){
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;

    if(fcntl(fd, F_SETLK, &fl) == -1){
        if(errno == EACCES || errno == EAGAIN){
            return -1;
        }
        perror("fcntl F_SETLK");
        return -1;
    }
    return 0;
}

int main(){
    int fd = open(LOCK_FILE, O_RDWR | O_CREAT, 0644);
    if(fd == -1){
        perror("open");
        return 1;
    }

    printf("try getting single_instance lock...\n");
    if(try_get_lock(fd) == -1){
        printf("FAILED! There are already examples in operation.\n");
        close(fd);
        return 1;
    }

    printf("SUCCESSFULLY!\n");
    sleep(10);
    printf("EXITED!\n");
    close(fd);
    return 0;
}