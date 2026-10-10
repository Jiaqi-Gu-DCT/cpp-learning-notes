#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
int GLOBAL = 1010;

int main(){
    pid_t pid = fork();
    if(pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if(pid == 0){
        printf("当前处于子进程： %d中，其所属父进程为： %d，开始执行。\n", getpid(), getppid());
        GLOBAL = 1011;
        sleep(2);
        printf("GLOBAL的值为： %d， 所处虚拟地址是：%p。\n", GLOBAL, &GLOBAL);
        printf("子进程： %d准备退出。\n", getpid());
        fflush(NULL);
        _exit(EXIT_SUCCESS);
    }

    if(pid > 0){
        printf("父进程： %d成功创建子进程： %d\n", getpid(), pid);
        GLOBAL = 1001;
        sleep(2);
        printf("GLOBAL的值为： %d， 所处虚拟地址是：%p。\n", GLOBAL, &GLOBAL);
        int status;
        pid_t pid_ret = waitpid(pid, &status, 0);
        if(pid_ret == -1){perror("waitpid"); exit(EXIT_FAILURE);}
        printf("子进程已经终止，状态码为：%d\n", status);
        if(WIFEXITED(status)){
            printf("正常退出，退出码：%d\n", WEXITSTATUS(status));
        }else if(WIFSIGNALED(status)){
            printf("被信号终止，终止信号：%d\n", WTERMSIG(status));
        }else if(WIFSTOPPED(status)){
            printf("进程被暂停，暂停信号：%d\n", WSTOPSIG(status));
        }
    }
    return 0;
}