// 直观对比阻塞读和非阻塞读的行为差异，理解O_NONBLOCK标志的作用，掌握用fcntl动态修改文件状态的标准写法。
// 用管道来演示：管道为空时，阻塞 read 会永久卡住；非阻塞 read 会立刻返回 -1 并设置 `EAGAIN`。
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

int main(){
    int pipefd[2];
    if(pipe(pipefd) == -1){
        perror("pipe");
        return 1;
    }

    printf("test 1: default Blocking mode\n");
    printf("pipe is empty, try reading...\n");

    printf("test 2: switch to Non-blocking mode\n");
    // 1. get original flag
    int flags = fcntl(pipefd[0], F_GETFL);
    if(flags == -1){
        perror("fcntl F_GETFL");
        return 1;
    }
    printf("original flag is 0x%x\n", flags);

    // 2.add to Non-blocking flag
    flags |= O_NONBLOCK;
    if(fcntl(pipefd[0], F_SETFL, flags) == -1){
        perror("fcntl F_SETFL");
        return 1;
    }
    printf("Non-blocking is set\n");

    // 3.read empty pipe at Non-blocking mode
    char buf[32];
    ssize_t n = read(pipefd[0], buf, sizeof(buf));
    printf("read's returned value is:%zd\n", n);
    if(n == -1){
        printf("errno = %d\n", errno);
        if(errno == EAGAIN){
            printf("As expected, no data is currently available for reading.\n");
        }
    }

    // 4.write to pipe
    printf("read at Non-blocking mode after writing.\n");
    if(write(pipefd[1], "aiueo", 6) == -1){
        perror("write");
        return 1;
    }
    n = read(pipefd[0], buf, sizeof(buf));
    printf("read's returned value is:%zd, content is:%.*s\n", n, (int)n, buf);

    close(pipefd[0]);
    close(pipefd[1]);
    return 0;
}