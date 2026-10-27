#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int main(){
    int pipefd[2];
    if(pipe(pipefd) == -1){perror("pipe"); exit(1);}
    pid_t pid = fork();
    if(pid < 0){perror("fork"); exit(1);}

    if(pid == 0){
        // 子进程
        close(pipefd[1]);
        char buf[1024];
        ssize_t wri = read(pipefd[0], buf, sizeof(buf) - 1 );
        if(wri < 0){perror("read"); _exit(1);}
        if(wri > 0){buf[wri] = '\0'; printf("子进程收到：%s\n", buf);}
        close(pipefd[0]);
        _exit(0);
    }else{
        // 父进程
        close(pipefd[0]);
        const char* msg = "舞幽壑之潜蛟，泣孤舟之嫠妇。";
        if(write(pipefd[1], msg, strlen(msg)) < 0){perror("write"); exit(1);}
        printf("父进程传入：%s\n", msg);
        close(pipefd[1]);
        wait(NULL);
    }
    return 0;
}