#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>

int main(void){
    int fd = open("lock.txt", O_WRONLY | O_CREAT | O_EXCL, 0644);
    if(fd == -1){
        if(errno == EEXIST){
            printf("file is existed! FAILED TO ACQUIRE LOCK! \n");
        }else{
            perror("open failed");
        }
        exit(EXIT_FAILURE);
    }

    printf("CREATE SUCCESSFULLY！\n");
    close(fd);
    return 0;
}