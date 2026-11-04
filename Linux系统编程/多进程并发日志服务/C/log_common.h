#ifndef LOG_COMMON_H
#define LOG_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

// ===== 配置参数 =====
#define KEY_PATH        "/tmp"
#define SHM_PROJ        'L'
#define SEM_PROJ        'G'
#define LOG_FILE        "/tmp/mylogger.log"
#define BUFFER_SIZE     1024    // 环形缓冲区大小(日志条目数)
#define MAX_MSG_LEN     512     // 单条日志最大长度
#define FLUSH_INTERVAL  2       // 落盘间隔(秒)
#define MAX_LOG_SIZE    (10*1024*1024) // 日志文件最大10MB

// ===== 日志级别 ===== 将匿名的枚举类型起一个别名（typedef），作为自定义的数据类型LogLevel
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO  = 1,
    LOG_WARN  = 2,
    LOG_ERROR = 3
} LogLevel;

static const char *LEVEL_NAMES[] = {"DEBUG", "INFO", "WARN", "ERROR"};

// ===== 日志条目结构体 ===== 按照功能需求中的日志格式构造一个结构体
typedef struct {
    char timestamp[32];   // 时间戳字符串
    char level[8];        // 级别字符串
    char proc_name[32];   // 进程名
    int  pid;             // 进程ID
    char message[MAX_MSG_LEN]; // 日志内容
} LogEntry;

// ===== 共享内存环形缓冲区 =====
typedef struct {
    LogEntry entries[BUFFER_SIZE]; // 纯数据无指针的结构体可以被这样定义为结构体数组
    int in;         // 写入位置
    int out;        // 读取位置
    int count;      // 当前条目数
    int shutdown;   // 关闭标志
} LogBuffer;

// ===== 信号量索引 =====
#define SEM_EMPTY   0
#define SEM_FULL    1
#define SEM_MUTEX   2

// ===== System V 信号量共用联合体 =====：控制操作必需的联合体
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// ===== 工具函数：P操作（阻塞） =====：static inline：头文件中定义通用工具函数
static inline void sem_p(int semid, int semnum) {
    struct sembuf op = {semnum, -1, 0};
    while (semop(semid, &op, 1) == -1) {
        if (errno != EINTR) {
            perror("sem_p");
            exit(1);
        }
    }
}

// ===== 工具函数：P操作（非阻塞） =====
static inline int sem_p_try(int semid, int semnum) {
    struct sembuf op = {semnum, -1, IPC_NOWAIT};
    return semop(semid, &op, 1);
}

// ===== 工具函数：V操作 =====
static inline void sem_v(int semid, int semnum) {
    struct sembuf op = {semnum, 1, 0};
    while (semop(semid, &op, 1) == -1) {
        if (errno != EINTR) {
            perror("sem_v");
            exit(1);
        }
    }
}

// ===== 工具函数：获取当前时间字符串 =====：数组作为函数参数传递时，只会传递首地址，不会复制整个数组。
static inline void get_timestamp(char *buf, size_t len) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", tm);
}

// ===== 创建/获取共享内存和信号量 =====
// create=1: 服务端创建模式；create=0: 客户端连接模式
static inline int init_ipc(int *shmid, int *semid, int create) {
    key_t shm_key = ftok(KEY_PATH, SHM_PROJ);
    key_t sem_key = ftok(KEY_PATH, SEM_PROJ);

    if (shm_key == -1 || sem_key == -1) {
        perror("ftok");
        return -1;
    }

    if (create) {
        // 创建共享内存，已存在则先删除再重建
        *shmid = shmget(shm_key, sizeof(LogBuffer), IPC_CREAT | IPC_EXCL | 0666);
        if (*shmid == -1) {
            if (errno == EEXIST) {
                int old = shmget(shm_key, sizeof(LogBuffer), 0666);
                if (old != -1) shmctl(old, IPC_RMID, NULL);
                *shmid = shmget(shm_key, sizeof(LogBuffer), IPC_CREAT | IPC_EXCL | 0666);
            }
            if (*shmid == -1) {
                perror("shmget create");
                return -1;
            }
        }

        // 创建信号量集，已存在则先删除再重建
        *semid = semget(sem_key, 3, IPC_CREAT | IPC_EXCL | 0666);
        if (*semid == -1) {
            if (errno == EEXIST) {
                int old = semget(sem_key, 3, 0666);
                if (old != -1) semctl(old, 0, IPC_RMID);
                *semid = semget(sem_key, 3, IPC_CREAT | IPC_EXCL | 0666);
            }
            if (*semid == -1) {
                perror("semget create");
                return -1;
            }
        }

        // 初始化信号量
        union semun arg;
        arg.val = BUFFER_SIZE; semctl(*semid, SEM_EMPTY, SETVAL, arg);
        arg.val = 0;           semctl(*semid, SEM_FULL,  SETVAL, arg);
        arg.val = 1;           semctl(*semid, SEM_MUTEX, SETVAL, arg);

        // 初始化共享内存
        LogBuffer *buf = (LogBuffer *)shmat(*shmid, NULL, 0);
        if (buf == (void *)-1) {
            perror("shmat init");
            return -1;
        }
        memset(buf, 0, sizeof(LogBuffer));
        shmdt(buf);
    } else {
        // 客户端模式：直接获取已有的IPC资源: flag这个参数不带IPC_CREAT的话就是从已有的内容中获取，如果不存在则返回-1
        *shmid = shmget(shm_key, sizeof(LogBuffer), 0666);
        *semid = semget(sem_key, 3, 0666);
        if (*shmid == -1 || *semid == -1) {
            fprintf(stderr, "日志服务未启动，请先启动logger\n");
            return -1;
        }
    }
    return 0;
}

#endif // LOG_COMMON_H
