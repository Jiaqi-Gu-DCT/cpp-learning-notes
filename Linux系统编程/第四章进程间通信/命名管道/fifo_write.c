#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FIFO_PATH "/tmp/myfifo"

int main(){
    if((mkfifo(FIFO_PATH, 0666)) == -1){
        // if file is existed, just skip it.
        if(errno != EEXIST){perror("mkfifo"); exit(EXIT_FAILURE);}
    }

    printf("waitting for the read end...\n");
    int fd = open(FIFO_PATH, O_WRONLY);
    if(fd == -1){perror("open"); exit(EXIT_FAILURE);}
    printf("the read end id connected!\n");
    

    const char* msg[] = {"hello", "KINGWA", "city."};
    for(int i = 0; i < 3; i++){
        int writed = write(fd, msg[i], strlen(msg[i]) + 1);
        if(writed == -1){perror("write"); break;}
        sleep(1);
    }

    close(fd);
    printf("the write end is exited.\n");
    return 0;

}