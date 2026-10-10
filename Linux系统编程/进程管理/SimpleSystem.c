// 请用 fork + exec + waitpid实现一个简化版的system()函数，要求： 
// ‑接收一个命令字符串（如"ls ‑l /tmp"） 
// ‑在子进程中通过/bin/sh ‑c执行该命令 
// ‑父进程阻塞等待命令执行完成 
// ‑返回命令的退出状态（与 system()一致：正常退出返回退出码，被信号终止返回128+信号号） 
// ‑ 正确处理 fork/exec/waitpid 的错误
// 能否在此基础上拓展支持 command > output.txt 格式的输出重定向？
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

int SimpleSystem(const char* command, const char* out_file){
    // 参数检查
    if(command == NULL){return 1;}
    pid_t pid = fork();
    // fork检查
    if(pid == -1){perror("fork"); return -1;}
    // 子进程逻辑
    if(pid == 0){
        printf("当前位于【子进程】：%d, 所属【父进程】为：%d\n", getpid(), getppid());
        if(out_file != NULL){
            int fd = open(out_file, O_WRONLY|O_CREAT|O_TRUNC, 0644);
            if(fd == -1){perror("open file"); _exit(127);}
            if((dup2(fd, STDOUT_FILENO)) == -1){perror("dup2"); _exit(127);}
            close(fd);
        }
        execl("/bin/sh", "sh", "-c", command, NULL);
        // execl正确不返回 错误才返回 走到这里一定出错了
        perror("execl /bin/sh");
        _exit(127);
    }
    // 父进程逻辑
    if(pid > 0){
        printf("当前位于【父进程】：%d，成功生成【子进程】：%d\n", getpid(), pid);
        int status;
        pid_t waiting = waitpid(pid, &status, 0);
        if(waiting == -1){perror("waitpid"); return -1;}
        if(WIFEXITED(status)){
            return WEXITSTATUS(status);
        }else if(WIFSIGNALED(status)){
            return 128 + WTERMSIG(status);
        }
    }
    return -1;
}

int main(){
    // 合法的命令
    int ret1 = SimpleSystem("ls -l", "out.txt");
    printf("命令的退出状态：%d\n", ret1);

    // 非法的命令
    int ret2 = SimpleSystem("壬戌之秋", NULL);
    printf("命令的退出状态：%d\n", ret2);

    return 0;
}