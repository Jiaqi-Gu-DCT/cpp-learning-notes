#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FIFO_PATH "/tmp/myfifo"
int main(){
    if((mkfifo(FIFO_PATH, 0666)) == -1){
        if(errno != EEXIST){perror("mkfifo"); exit(EXIT_FAILURE);}
    }

    printf("waitting for the write end...\n");
    int fd = open(FIFO_PATH, O_RDONLY);
    if(fd == -1){perror("open"); exit(EXIT_FAILURE);}
    printf("the write end is connected!\n");

    char buf[256];
    ssize_t n;
    while((n = (read(fd, buf, sizeof(buf) - 1))) > 0){
        buf[n] = '\0';
        printf("received “%s”\n", buf);
    }

    close(fd);
    printf("the read end is exited.\n");
    return 0;
}