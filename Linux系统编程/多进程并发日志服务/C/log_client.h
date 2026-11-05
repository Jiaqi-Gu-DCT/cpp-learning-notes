#ifndef LOG_CLIENT_H
#define LOG_CLIENT_H

#include "log_common.h"

// 客户端上下文
typedef struct {
    int shmid;
    int semid;
    LogBuffer *buf;
    char proc_name[32];// 进程名，是日志格式中的一部分，用于提示读者这个业务进程的逻辑名称
    int connected;
} LogClient;

// 连接日志服务
int log_connect(LogClient *client, const char *proc_name);

// 阻塞写入日志
int log_write(LogClient *client, LogLevel level, const char *fmt, ...);

// 非阻塞写入日志（缓冲区满则丢弃）
int log_write_nb(LogClient *client, LogLevel level, const char *fmt, ...);

// 断开连接
void log_disconnect(LogClient *client);

#endif
