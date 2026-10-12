// 请编写一个守护进程程序，要求： 
// ‑完整实现守护进程化的所有步骤 ‑守护进程每分钟向日志文件/tmp/mydaemon.log写入当前时间和进程信息
// ‑ 支持通过 kill 发送 SIGTERM 信号优雅退出（退出前写一条日志） ‑ 写一个启动脚本或说明如何在后台运行和停止
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>

#define LOG_FILE_PATH "/tmp/mydaemon.log"
#define PID_FILE_PATH "/var/run/mydaemon.pid"
// static： 限定该变量只能在本文件使用； volatile：强制读内存； sig_atomic_t：原子读与写
static volatile sig_atomic_t running = 1;

// 写入日志文件
void writeLog(const char* format, ...){
    FILE* fp = fopen(LOG_FILE_PATH, "a");
    if(fp == NULL){return;}

    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char buf_time[64];
    strftime(buf_time, sizeof(buf_time), "%Y-%m-%d %H:%M:%S", tm);
    fprintf(fp, "[%s] [PID=%d]", buf_time, getpid());

    va_list args;
    // 把收纳袋对准format这个格式字符串的后面，从第一个可变参数开始装。
    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);

    fprintf(fp, "\n");
    fflush(fp);
    fclose(fp);
}

// 信号处理函数
void signalHandler(int sig){
    switch(sig){
        case SIGTERM: // 执行 kill进程PID 时默认发送的信号
            writeLog("received SIGTERM, preparing to exit...");
            running = 0;
            break; 
        case SIGINT: // 终端按 Ctrl+C 时发送
            writeLog("received SIGINT, preparing to exit...");
            running = 0;
            break;
        case SIGHUP:
            writeLog("received SIGHUP, reloading configuration...");
            break;
        default:
            writeLog("received unknown signal %d, ignoring", sig);
            break;
    }
}

// 注册信号处理函数：告知操作系统，进程收到哪些信号时要执行自定义逻辑、哪些信号直接忽略。
void setupSignals(){
    // 规则の义 定
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signalHandler;
    // 处理信号的时候不需要屏蔽任何信号（因为我们的信号处理函数很轻量）
    sigemptyset(&sa.sa_mask);
    // 不采用SA_RESTART标志，避免某些系统调用被信号中断后自动重启
    sa.sa_flags = 0;

    // 注册需要处理的信号（按照制定的规则sa，NULL：不保留旧规则）
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);

    // 忽略一些可能导致进程终止的信号
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

}


// 守护进程
int myDaemon(){
    // 1.创建父进程 爷进程退出
    pid_t pid = fork();
    if(pid < 0){perror("fork g"); return -1;}
    if(pid > 0){exit(EXIT_SUCCESS);}

    // 2.创建新会话，脱离控制终端
    if(setsid() < 0){perror("setsid"); return -1;}

    // 3.创建子进程，父进程退出
    pid = fork();
    if(pid < 0){perror("fork f"); return -1;}
    if(pid > 0){exit(EXIT_SUCCESS);}

    // 4.改变工作目录到根目录
    if((chdir("/")) < 0){perror("chdir"); return -1;}

    // 5.重设文件权限掩码
    umask(0);

    // 6.关闭所有继承的文件描述符
    int max_fd = sysconf(_SC_OPEN_MAX);
    if(max_fd < 0) max_fd = 1024;
    for(int i = 0; i < max_fd; i++){close(i);}

    // 7.重定向stdin/stdout/stderr到 /dev/null
    int fd0 = open("/dev/null", O_RDWR);
    int fd1 = dup(0);
    int fd2 = dup(0);

    if(fd0 != 0 || fd1 != 1 || fd2 != 2){
        writeLog("unexpected file descriptors %d %d %d", fd0, fd1, fd2);
        return -1;
    }
    return 0;
}

// 写入pid文件
int writePidFile(){
    FILE* fp = fopen(PID_FILE_PATH, "w");
    if(fp == NULL){
        // 可能是权限问题
        fp = fopen("/tmp/mydaemon.pid", "w");
        if(fp == NULL){
            return -1;
        }
    }
    fprintf(fp, "%d\n", getpid());
    fclose(fp);
    return 0;
}

// 删除pid文件
void removePidFile(){
    // int unlink(const char *pathname); 只能删除文件，不能删除目录
    unlink(PID_FILE_PATH);
    unlink("/tmp/mydaemon.pid");
}

int main(int argc, char* argv[]){
    int foreground = 0;
    if(argc > 1 && strcmp(argv[1], "-f") == 0){
        foreground = 1;
    }
    if(!foreground){
        if(myDaemon() < 0){
            exit(EXIT_FAILURE);
        } 
    }

    setupSignals();
    writePidFile();
    writeLog("守护进程启动！");
    writeLog("PID: %d, PPID: %d, SID: %d", getpid(), getppid(), getsid(0));

    int counter = 0;
    while(running){
        counter++;
        writeLog("心跳：%d  守护进程运行中...", counter);

        // 睡眠 60 秒，但用 sleep 会被信号中断
        // 被信号中断后，sleep 返回剩余秒数
        // 我们用循环确保睡满 60 秒（除非 running 变为 0）
        int remaining = 60;
        while(remaining > 0 && running){
            remaining = sleep(remaining);
        }
    }

    writeLog("守护进程共计运行了：%d个周期", counter);
    writeLog("守护进程退出！");
    removePidFile();
    return 0;
}