// 多个进程需要同时读写同一个数据文件data.db，请设计一个方案： 
// ‑多个进程可以同时读取（读锁共享） 
// ‑任何时刻只能有一个进程写入（写锁独占）
// ‑ 写锁和读锁互斥 
// ‑ 使用 fcntl 记录锁实现
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

// 获取当前时间字符串
const char* now(){
    static char buf[64];
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    strftime(buf, sizeof(buf), "%H:%M:%S", tm);
    return buf;
}

// 加读锁(共享锁)，阻塞等待
int lockRead(int fd){
    struct flock fl_read;
    fl_read.l_type = F_RDLCK;
    fl_read.l_whence = SEEK_SET;
    fl_read.l_start = 0;
    fl_read.l_len = 0;
    fl_read.l_pid = getpid();

    printf("[%s] PID:%d 正在尝试加 读锁\n", now(), fl_read.l_pid);
    if((fcntl(fd, F_SETLK, &fl_read)) == -1){perror("SETLK"); return -1;}
    printf("[%s] PID:%d 已经获取到 读锁\n", now(), fl_read.l_pid);
    return 0;
}

// 加写锁(独占锁)，阻塞等待
int lockWrite(int fd){
    struct flock fl_write;
    fl_write.l_type = F_WRLCK;
    fl_write.l_whence = SEEK_SET;
    fl_write.l_start = 0;
    fl_write.l_len = 0;
    fl_write.l_pid = getpid();

    printf("[%s] PID:%d 正在尝试加 写锁\n", now(), fl_write.l_pid);
    if((fcntl(fd, F_SETLK, &fl_write)) == -1){perror("SETLK"); return -1;}
    printf("[%s] PID:%d 已经获取到 写锁\n", now(), fl_write.l_pid);
    return 0;
}

// 解锁
int unlock(int fd){
    struct flock fl_un;
    fl_un.l_type = F_UNLCK;
    fl_un.l_whence = SEEK_SET;
    fl_un.l_start = 0;
    fl_un.l_len = 0;
    fl_un.l_pid = getpid();

    printf("[%s] PID:%d 正在尝试解锁\n", now(), fl_un.l_pid);
    if((fcntl(fd, F_UNLCK, &fl_un)) == -1){perror("UNLK"); return -1;}
    printf("[%s] PID:%d 已经完成解锁\n", now(), fl_un.l_pid);
    return 0;
}

// 读文件
void readFile(int fd){
    if(lockRead(fd) == -1){perror("lockRead"); exit(1);}

    printf("[%s] PID:%d 正在读取文件\n", now(), getpid());
    char buf[1024];
    lseek(fd, 0, SEEK_SET);
    ssize_t nread;
    if((nread = (read(fd, buf, 1023))) > 0){
        printf("[%s] PID:%d 读取内容: %s\n", now(), getpid(), buf);
    }else if (nread == 0){
        printf("[%s] PID:%d 该文件无内容\n", now(), getpid());
    }else{perror("read"); exit(1);}

    sleep(3);
    printf("[%s] PID:%d 读取完成\n", now(), getpid());
    unlock(fd);
}

// 写文件
void writeFile(int fd, const char* data){
    if(lockWrite(fd) == -1){perror("lockWrite"); exit(1);}
    printf("[%s] PID:%d 正在写入文件\n", now(), getpid());
    lseek(fd, 0, SEEK_SET);
    // 清空文件内容
    ftruncate(fd, 0);
    ssize_t nwrite = write(fd, data, strlen(data));
    if(nwrite == -1){perror("write"); exit(1);}
    fsync(fd);

    sleep(3);
    printf("[%s] PID:%d 写入完成: %s\n", now(), getpid(), data);
    unlock(fd);
}

int main(int argc, char* argv[]){
    if(argc < 2){
        fprintf(stderr, "Usage: %s <read|write>\n", argv[0]);
        exit(1);
    }
    int fd = open("data.db", O_RDWR|O_CREAT, 0644);
    if(fd == -1){perror("open"); exit(1);}

    if(strcmp(argv[1], "read") == 0){
        readFile(fd);
    }else if(strcmp(argv[1], "write") == 0){
        if(argc < 3){
            fprintf(stderr, "Usage: %s write <data>\n", argv[0]);
            exit(1);
        }
        writeFile(fd, argv[2]);
    }else{
        fprintf(stderr, "未知操作: %s！\n", argv[1]);
        close(fd);
        exit(1);
    }
    close(fd);
    return 0;
}