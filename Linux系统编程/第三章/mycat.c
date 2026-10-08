// 请用 Linux系统调用（open/read/write/close）实现一个简化版的cat命令，要求：
// ‑支持从命令行参数指定的文件读取内容并输出到标准输出
// ‑ 如果没有指定文件，则从标准输入读取（实现 cat 的默认行为） 
// ‑ 正确处理错误情况,缓冲区大小设为 4096 字节
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

int mycat(int fd){
    char buf[4096];
    ssize_t nread;
    while(1){
        nread = read(fd, buf, sizeof(buf));
        if(nread < 0){
            if(errno == EINTR) continue;
            perror("read");
            return -1;
        }
        if(nread == 0) break;
        ssize_t written = 0;
        while(written < nread){
            ssize_t nwrite = write(STDOUT_FILENO, buf + written, nread - written);
            if(nwrite == -1){
                if(errno == EINTR) continue;
                perror("write");
                return -1;
            }
            written += nwrite;
        }
    }
    return 0;
}

int main(int argc, char* argv[]){
    int exit_status = EXIT_SUCCESS;
    if(argc == 1){
        // 无指定文件，从标准输入中导入
        if(mycat(STDIN_FILENO) < 0){
            exit_status = EXIT_FAILURE;
        }
        return exit_status;
    }
    for(int i = 1; i < argc; i++){
        int fd = open(argv[i], O_RDONLY);
        if(fd == -1){
            perror(argv[i]);
            exit_status = EXIT_FAILURE;
            continue;
        }
        if(mycat(fd) < 0){
            fprintf(stderr, "%s: 读取失败\n", argv[i]);
            exit_status = EXIT_FAILURE;
            close(fd);
            continue;
        }
        close(fd);
    }
    return exit_status;
}