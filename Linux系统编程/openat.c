#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>

int main(void){
    int dir_fd = open("/home/sgaeyoco/桌面/cpp/Linux系统编程/test", O_RDONLY | O_DIRECTORY);
    if(dir_fd == -1){
        perror("open /home/sgaeyoco/桌面/cpp/Linux系统编程/test failed");
        exit(EXIT_FAILURE);
    }

    int fd = openat(dir_fd, "data.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd == -1){
        perror("open data.txt failed");
        close(dir_fd);
        exit(EXIT_FAILURE);
    }

    write(fd, "test content\n", 20);

    close(fd);
    close(dir_fd);
    printf("DONE\n");
    return 0;
}