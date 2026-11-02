#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

static volatile sig_atomic_t g_shutdown = 0;

void signalHandler(int signum){
    const char* msg;
    switch(signum){
        case SIGTERM: msg = "\n[SIGTERM] 准备优雅退出。\n"; break;
        case SIGINT: msg = "\n[SIGINT] 准备优雅退出。\n"; break;
        case SIGHUP: msg = "\n[SIGHUP] 准备重新加载配置。\n"; break;
        default: msg = "\n收到未知的信号。\n"; break;
    }
    write(STDERR_FILENO, msg, strlen(msg));
    if(signum == SIGTERM || signum == SIGINT){g_shutdown = 1;}
}

int main(){
    printf("服务器启动，PID = %d\n", getpid());

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);

    signal(SIGPIPE, SIG_IGN);

    int counter = 0;
    while(!g_shutdown){
        counter++;
        printf("第%d次循环\n", counter);
        sleep(1);
    }

    printf("正在清理资源\n");
    printf("服务器安全退出\n");
    return 0;
}