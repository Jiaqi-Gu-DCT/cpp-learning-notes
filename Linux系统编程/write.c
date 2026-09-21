#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

ssize_t write_n(int fd, const void* buf, size_t n){
    size_t total = 0;
    ssize_t nwrite;
    char* p = (char *)buf;

    while(total < n){
        nwrite = write(fd, p + total, n - total);
        if(nwrite == -1){
            if(errno == EINTR) continue;
            return -1;
        }
        total += nwrite;
    }
    return total;
}

int main(){
    int fd = open("test/write.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fd == -1){
        perror("open");
        return 1;
    }

    const char* message = "Kingho University Computer college\n";
    if(write_n(fd, message, strlen(message)) == -1){
        perror("write");
        close(fd);
        return 1;
    }
    printf("写入文件成功！\n");
    close(fd);
    return 0;

}