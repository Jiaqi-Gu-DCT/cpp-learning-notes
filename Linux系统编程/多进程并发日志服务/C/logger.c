#define _GNU_SOURCE
#include "log_common.h"

static volatile sig_atomic_t g_shutdown = 0;
static int g_shmid = -1, g_semid = -1;
static LogBuffer *g_buf = NULL;

// 信号处理函数
void signal_handler(int sig) {
    const char *msg = "\n[Logger] 收到退出信号，准备优雅关闭...\n";
    write(STDERR_FILENO, msg, strlen(msg));
    g_shutdown = 1;
}

// 返回值：1表示发生了轮转，0表示未轮转，-1表示出错
int rotate_log_if_needed(int *log_fd) {
    struct stat st;
    if (stat(LOG_FILE, &st) != 0 || st.st_size <= MAX_LOG_SIZE) {
        return 0; // 不需要轮转
    }

    char old_path[256];
    snprintf(old_path, sizeof(old_path), "%s.1", LOG_FILE);
    
    // 1. 重命名旧文件
    if (rename(LOG_FILE, old_path) != 0) {
        perror("rename log file");
        return -1;
    }
    printf("[Logger] 日志轮转: %s -> %s\n", LOG_FILE, old_path);

    // 2. 关闭旧文件描述符
    close(*log_fd);

    // 3. 重新创建并打开新的日志文件
    int new_fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (new_fd == -1) {
        perror("reopen log file");
        return -1;
    }

    // 4. 把新fd写回外部变量
    close(*log_fd);
    *log_fd = new_fd;
    return 1;
}


// 批量落盘，返回写入的日志条数
int flush_buffer(int fd) {
    int flushed = 0;
    while (1) {
        // 非阻塞尝试获取满槽位
        if (sem_p_try(g_semid, SEM_FULL) == -1) {
            break; // 缓冲区空
        }

        sem_p(g_semid, SEM_MUTEX);
        LogEntry *entry = &g_buf->entries[g_buf->out];
        g_buf->out = (g_buf->out + 1) % BUFFER_SIZE;
        g_buf->count--;
        sem_v(g_semid, SEM_MUTEX);
        sem_v(g_semid, SEM_EMPTY);

        // 格式化并写入文件
        char line[MAX_MSG_LEN + 256];
        int len = snprintf(line, sizeof(line),
                           "[%s] [%s] [%s:%d] %s\n",
                           entry->timestamp, entry->level,
                           entry->proc_name, entry->pid,
                           entry->message);
        write(fd, line, len);
        flushed++;
    }

    if (flushed > 0) {
        fsync(fd);
    }
    return flushed;
}

int main() {
    printf("========== 多进程日志服务(Logger)启动 ==========\n");
    printf("共享内存缓冲区大小: %d 条\n", BUFFER_SIZE);
    printf("日志文件: %s\n", LOG_FILE);
    printf("落盘间隔: %d 秒\n\n", FLUSH_INTERVAL);

    // 注册信号处理
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    signal(SIGPIPE, SIG_IGN);

    // 初始化IPC（创建模式）
    if (init_ipc(&g_shmid, &g_semid, 1) != 0) {
        fprintf(stderr, "初始化IPC失败\n");
        return 1;
    }

    // 映射共享内存
    g_buf = (LogBuffer *)shmat(g_shmid, NULL, 0);
    if (g_buf == (void *)-1) {
        perror("shmat logger");
        return 1;
    }

    // 打开日志文件
    int log_fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd == -1) {
        perror("open log file");
        return 1;
    }

    printf("[Logger] 就绪，等待前端进程提交日志...\n");
    printf("[Logger] 发送SIGTERM/SIGINT 优雅退出\n\n");

    // 主循环：定时落盘
    while (!g_shutdown) {
        int remaining = FLUSH_INTERVAL;
        while (remaining > 0 && !g_shutdown) {
            remaining = sleep(remaining);
        }

        rotate_log_if_needed(&log_fd);
        int n = flush_buffer(log_fd);
        if (n > 0) {
            printf("[Logger] 落盘%d条日志，缓冲区剩余%d条\n", n, g_buf->count);
        }
    }

    // ===== 优雅退出 =====
    printf("\n[Logger] 优雅关闭中，将缓冲区剩余日志全部落盘...\n");

    // 设置关闭标志，通知前端停止写入
    sem_p(g_semid, SEM_MUTEX);
    g_buf->shutdown = 1;
    sem_v(g_semid, SEM_MUTEX);

    // 唤醒所有阻塞在empty上的前端进程
    // 唤醒阻塞客户端，是为了不让它们卡死，它们不会产生新日志
    for (int i = 0; i < BUFFER_SIZE; i++) {
        sem_v(g_semid, SEM_EMPTY);
    }

    // 最后一次全量落盘
    // 为了把缓冲区里已经存在的收尾日志全部清空，读的依然是环形缓冲区
    // 循环最后一次落盘之后，到设置shutdown之前，客户端成功写入的日志；
    // 设置shutdown的瞬间，已经走到半路上（申请完槽位、正在写入）的客户端，它们会完成最后这一次写入，然后才会被后续的新请求拦截。
    int total = 0;
    int n;
    while ((n = flush_buffer(log_fd)) > 0) {
        total += n;
    }
    printf("[Logger] 退出前额外落盘%d条日志\n", total);

    // 资源清理
    close(log_fd);
    shmdt(g_buf);
    shmctl(g_shmid, IPC_RMID, NULL);
    semctl(g_semid, 0, IPC_RMID);

    printf("[Logger] 资源已清理，服务已退出\n");
    printf("========== Logger 结束 ==========\n");
    return 0;
}
