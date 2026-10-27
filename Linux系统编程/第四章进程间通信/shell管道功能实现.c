#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int main(){
    int pipefd[2];
    if(pipe(pipefd) == -1){perror("pipe"); exit(EXIT_FAILURE);}

    pid_t pid1 = fork();
    if(pid1 == 0){
        // 子进程1：ls -l
        close(pipefd[0]);
        // dup2的第二个参数：希望被替换的文件描述符
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        char* const args[] = {"ls", NULL};
        execvp(args[0], args);
        perror("execvp ls");
        _exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if(pid2 == 0){
        // 子进程2： wc -l
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        char* const args[] = {"wc", "-l", NULL};
        execvp(args[0], args);
        perror("execvp wc");
        _exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    return 0;
}