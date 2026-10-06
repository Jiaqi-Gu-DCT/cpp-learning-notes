// 1. 创建一个文件，写入字符串 `"0123456789abcdef"`（共 16 字节）
// 2. 用 lseek 获取当前文件偏移量，打印出来
// 3. 用 lseek 跳到文件第 5 字节的位置
// 4. 从该位置读取 4 个字节并打印
// 5. 用 lseek 跳到文件末尾，获取文件总大小并打印
// 6. 最后把指针重新回到文件开头
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

int main(){

    // char that need input
    char tmp[5];
    const char* msg = "0123456789abcdef";

    // create a file
    int fd = open("test/lseek.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(fd == -1){
        perror("open");
        return 1;
    }
    ssize_t m = write(fd, msg, strlen(msg));
    if(m == -1){
        perror("write");
        close(fd);
        return 1;
    }

    // get the offset of file currently
    off_t oft = lseek(fd, 0, SEEK_CUR);
    printf("当前文件偏移量为：%ld。", oft);

    // jump to the fifth byte
    lseek(fd, 5, SEEK_SET);

    ssize_t n = read(fd, tmp, 4);
    tmp[n] = '\0';
    printf("读取4字节：%s\n", tmp);
    
    off_t size = lseek(fd, 0, SEEK_END);
    printf("文件总大小为：%ld\n", size);
    lseek(fd, 0, SEEK_SET);
    close(fd);

    return 0;
}