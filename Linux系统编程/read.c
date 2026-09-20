#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define BUF_SIZE 1752

// 实际读取到的字节数
ssize_t read_n(int fd, void* buf, size_t n){
    size_t total = 0;
    ssize_t nread;
    char* p = (char*) buf;

    while(total < n){
        nread = read(fd, p + total, n - total);
        if(nread == 0) break;
        if(nread == -1){
            if(errno == EINTR) continue;
            return -1;
        }
        total += nread;
    }
    return total;
}

int main(){
    int fd = open("test/text.txt", O_RDONLY);
    if(fd == -1){
        perror("open");
        return 1;
    }

    char buf[BUF_SIZE + 1];
    ssize_t n;

    while((n = read_n(fd, buf, BUF_SIZE)) > 0){
        buf[n] = '\0';
        printf("读取到了%zd个字节，内容：%s\n", n, buf);
    }

    if(n == -1){
        perror("read");
        close(fd);
        return 1;
    }

    printf("文件读取结束！\n");
    close(fd);
    return 0;

}