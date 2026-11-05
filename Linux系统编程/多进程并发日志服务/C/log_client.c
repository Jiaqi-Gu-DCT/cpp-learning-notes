#include "log_client.h"
#include <stdarg.h>

int log_connect(LogClient *client, const char *proc_name) {
    memset(client, 0, sizeof(LogClient));
    if (init_ipc(&client->shmid, &client->semid, 0) != 0) {
        return -1;
    }

    client->buf = (LogBuffer *)shmat(client->shmid, NULL, 0);
    if (client->buf == (void *)-1) {
        perror("shmat client");
        return -1;
    }

    strncpy(client->proc_name, proc_name, sizeof(client->proc_name) - 1);
    client->connected = 1;
    return 0;
}

// 内部通用写入函数
static int do_log_write(LogClient *client, LogLevel level,
                        const char *fmt, va_list ap, int nonblock) {
    if (!client->connected) return -1;

    // 构造日志条目
    LogEntry entry;
    memset(&entry, 0, sizeof(entry));
    get_timestamp(entry.timestamp, sizeof(entry.timestamp));
    strncpy(entry.level, LEVEL_NAMES[level], sizeof(entry.level) - 1);
    strncpy(entry.proc_name, client->proc_name, sizeof(entry.proc_name) - 1);
    entry.pid = getpid();
    // vsnprintf是snprintf的可变参数版本，它会自动遍历va_list ap里的所有参数，按照fmt格式字符串，
    // 把最终的日志消息拼好写入entry.message
    vsnprintf(entry.message, sizeof(entry.message), fmt, ap);

    // 申请空槽位
    if (nonblock) {
        if (sem_p_try(client->semid, SEM_EMPTY) == -1) { // 非阻塞形式
            return -1; // 缓冲区满，丢弃
        }
    } else { // 阻塞模式
        sem_p(client->semid, SEM_EMPTY);
    }

    // 互斥访问缓冲区
    sem_p(client->semid, SEM_MUTEX);

    // 检查服务是否已关闭
    if (client->buf->shutdown) {
        sem_v(client->semid, SEM_MUTEX);
        sem_v(client->semid, SEM_EMPTY);
        return -1;
    }

    // 写入环形缓冲区
    client->buf->entries[client->buf->in] = entry;
    client->buf->in = (client->buf->in + 1) % BUFFER_SIZE;
    client->buf->count++;

    sem_v(client->semid, SEM_MUTEX);
    sem_v(client->semid, SEM_FULL);

    return 0;
}

int log_write(LogClient *client, LogLevel level, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = do_log_write(client, level, fmt, ap, 0);
    va_end(ap);
    return ret;
}

int log_write_nb(LogClient *client, LogLevel level, const char *fmt, ...) {
    va_list ap;// 1. 定义一个可变参数列表变量
    va_start(ap, fmt);// 2. 初始化，让ap指向fmt后面的第一个可变参数
    int ret = do_log_write(client, level, fmt, ap, 1);
    va_end(ap);// 清理
    return ret;
}

void log_disconnect(LogClient *client) {
    if (client->connected && client->buf != (void *)-1) {
        shmdt(client->buf);
    }
    client->connected = 0;
}
