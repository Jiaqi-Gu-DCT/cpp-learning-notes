// 请分别用以下四种方式实现文件拷贝，目的是磁盘上出现两份独立的数据、两个独立的 inode
// 并对一个100MB的文件进行性能测试，分析差异原因：(a) 传统 read + write
// 方式（缓冲区 4096 字节） (b) mmap 内存映射方式 (c) sendfile 零拷贝方式 (d)splice
// 要求记录每种方式的耗时，并从内核数据拷贝次数的角度解释性能差异。

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/sendfile.h>
#include <errno.h>

#define BUF_SIZE 4096

long long get_time_us(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long) tv.tv_sec * 1000000 + tv.tv_usec;
}

// 方式1：传统 read + write
int CopyReadWrite(const char* src, const char* dst){
    int src_fd = open(src, O_RDONLY);
    if(src_fd == -1){perror("open src"); return -1;}

    int dst_fd = open(dst, O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if(dst_fd == -1){perror("open dst"); return -1;}

    ssize_t n;
    char buf[BUF_SIZE];
    while((n = read(src_fd, buf, BUF_SIZE)) > 0){
        ssize_t written = 0;
        while(written < n){
            ssize_t w = write(dst_fd, buf + written, n - written);
            if(w == -1){
                if(errno == EINTR) continue;
                perror("write dst");
                close(src_fd); close(dst_fd);
                return -1;
            }
            written += w;
        }
    }
    close(src_fd); close(dst_fd);
    return 0;
}

// 方式2：mmap内存映射
int CopyMmap(const char* src, const char* dst){
    int src_fd = open(src, O_RDONLY);
    if(src_fd == -1){perror("open src"); return -1;}

    struct stat st;
    if(fstat(src_fd, &st) == -1){perror("fstat"); close(src_fd); return-1;}
    size_t size = st.st_size;

    int dst_fd = open(dst, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(dst_fd == -1){perror("open dst"); return -1;}

    if(ftruncate(dst_fd, size) == -1){perror("ftruncate dst"); close(src_fd); close(dst_fd); return -1;}

    char* src_addr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    if(src_addr == MAP_FAILED){perror("src mmap"); close(src_fd); close(dst_fd); return -1;}

    char* dst_addr = mmap(NULL, size, PROT_WRITE, MAP_SHARED, dst_fd, 0);
    if(dst_addr == MAP_FAILED){perror("dst mmap"); close(src_fd); close(dst_fd); return -1;}

    memcpy(dst_addr, src_addr, size);
    msync(dst_addr, size, MS_SYNC);

    munmap(src_addr, size); munmap(dst_addr, size);
    close(src_fd); close(dst_fd);
    return 0;
}

// 方式3：sendfile
int CopySendfile(const char* src, const char* dst){
    int src_fd = open(src, O_RDONLY);
    if(src_fd == -1){perror("open src"); return -1;}

    struct stat st;
    if(fstat(src_fd, &st) == -1){perror("fstat src"); close(src_fd); return -1;}
    size_t size = st.st_size;

    int dst_fd = open(dst, O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if(dst_fd == -1){perror("open dst"); return -1;}

    off_t offset = 0;
    ssize_t ret = sendfile(dst_fd, src_fd, &offset, size);
    if(ret == -1){perror("sendfile"); close(src_fd); close(dst_fd); return -1;}

    fsync(dst_fd);
    close(src_fd);
    close(dst_fd);
    return 0;
}

// 方式4：splice
int CopySplice(const char* src, const char* dst){
    int src_fd = open(src, O_RDONLY);
    if(src_fd == -1){perror("open src"); return -1;}

    struct stat st;
    if(fstat(src_fd, &st) == -1){perror("fstat src"); close(src_fd); return -1;}
    size_t size = st.st_size;

    int dst_fd = open(dst, O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if(dst_fd == -1){perror("open dst"); return -1;}

    int pipefd[2]; pipe(pipefd);

    size_t bytes_left = size;
    while (bytes_left > 0) {
        // 管道最大只有64KB = 64*1024 = 65536
        size_t chunk = (bytes_left > 65536) ? 65536 : bytes_left;
        ssize_t s_in = splice(src_fd, NULL, pipefd[1], NULL, chunk, 0);
        if(s_in < 0){perror("splice s_in"); close(src_fd); close(dst_fd); return -1; }
        if(s_in == 0) break;
        size_t s_out = splice(pipefd[0], NULL, dst_fd, NULL, s_in, 0);
        if(s_out < 0){perror("splice s_out"); close(src_fd); close(dst_fd); return -1; }
        if(s_out == 0) break;
        bytes_left -= s_out;
    }   

    fsync(dst_fd);
    close(src_fd); close(dst_fd);
    return 0;
}

int main(int argc, char* argv[]){
    if(argc != 3){
        fprintf(stderr, "Usage: %s <srcfile> <dst_prefix>\n", argv[0]);
        fprintf(stderr, "Creates <dst_prefix>.rw, <dst_prefix>.mmap, <dst_prefix>.sendfile, <dst_prefix>.splice\n");
    }

    const char* src = argv[1];
    char dst[256];

    snprintf(dst, sizeof(dst), "%s.rw", argv[2]);
    long long t1 = get_time_us();
    if(CopyReadWrite(src, dst) == -1) exit(1);
    long long t2 = get_time_us();
    printf("read + write: %8.3f ms\n", (t2-t1) / 1000.0);

    snprintf(dst, sizeof(dst), "%s.mmap", argv[2]);
    t1 = get_time_us();
    if(CopyMmap(src, dst) == -1) exit(1);
    t2 = get_time_us();
    printf("mmap: %8.3f ms\n", (t2-t1) / 1000.0);

    snprintf(dst, sizeof(dst), "%s.sendfile", argv[2]);
    t1 = get_time_us();
    if(CopySendfile(src, dst) == -1) exit(1);
    t2 = get_time_us();
    printf("sendfile: %8.3f ms\n", (t2-t1) / 1000.0);

    snprintf(dst, sizeof(dst), "%s.splice", argv[2]);
    t1 = get_time_us();
    if(CopySplice(src, dst) == -1) exit(1);
    t2 = get_time_us();
    printf("splice: %8.3f ms\n", (t2-t1) / 1000.0);

    return 0;
}