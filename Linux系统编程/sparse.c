// 1. 创建一个新文件
// 2. 用 lseek 直接跳到 1MB（1024*1024）的位置
// 3. 只写入 1 个字节 `'a'`
// 4. 分别用 `stat` 命令和 `du` 命令查看文件，对比「逻辑大小」和「实际磁盘占用」
// 5. 再用 lseek 跳回到文件中间（500KB 位置），读取 1 个字节，看看值是什么
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

int main(){
    int fd = open("test/sparse.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(fd == -1){
        perror("open");
        return 1;
    }

    lseek(fd, 1024*1024, SEEK_SET);
    ssize_t l = write(fd, "a", 1);
    if(l == -1){
        perror("write");
        close(fd);
        return 1;
    }

    char tmp;
    lseek(fd, 512*1024, SEEK_SET);
    ssize_t m = read(fd, &tmp, 1);
    if(m == -1){
        perror("read");
        close(fd);
        return 1;
    }
    printf("文件中间读取到的值为：%c\n", tmp);
    printf("对应的 ASCII 码值：%d\n", (unsigned char)tmp);
    close(fd);
    return 0;
}