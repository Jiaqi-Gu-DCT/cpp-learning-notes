#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

int main(int argc, char* argv[]){
    if(argc!= 3){
        fprintf(stderr, "Usage:%s<src> <dst>\n", argv[0]);
        exit(1);
    }

    // 打开源文件
    int src_fd = open(argv[1], O_RDONLY);
    if(src_fd == -1) {perror("open src"); exit(1);}

    // 获取源文件大小
    struct stat st;
    if(fstat(src_fd, &st) == -1){perror("fstat"); exit(1);}
    size_t size = st.st_size;

    // 打开目标文件(创建并截断)
    int dst_fd = open(argv[2], O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(dst_fd == -1){perror("open dst"); exit(1);}

    // 必须先扩展目标文件大小，否则mmap写会触发SIGBUS
    if(ftruncate(dst_fd, size) == -1){perror("ftruncate"); exit(1);}

    // 映射源文件（只读，私有）
    char* src_addr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    if(src_addr == MAP_FAILED){perror("mmap src"); exit(1);}
    
    // 映射目标文件（读写，共享）
    char* dst_addr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, dst_fd, 0);
    if(dst_addr == MAP_FAILED){perror("mmap dst"); exit(1);}

    // 直接内存拷贝，像操作内存一样操作文件
    memcpy(dst_addr, src_addr, size);

    // 同步到磁盘，确保落盘
    msync(dst_addr, size, MS_SYNC);

    // 清理
    munmap(src_addr, size);
    munmap(dst_addr, size);
    close(src_fd);
    close(dst_fd);

    printf("Copy completed: %zu bytes\n", size);
    return 0;
}