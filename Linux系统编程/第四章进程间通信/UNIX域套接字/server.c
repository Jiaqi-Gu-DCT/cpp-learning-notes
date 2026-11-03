#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/unix_demo.sock"
#define BUF_SIZE 256

int main(){
    // 1.创建socket
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(server_fd == -1){perror("socket"); exit(EXIT_FAILURE);}
    // 2.配置地址
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(SOCKET_PATH)-1);
    // 3.（关键）删除旧的socket文件，否则bind将报告地址已被占用
    unlink(SOCKET_PATH);
    // 4.绑定+监听
    if(bind(server_fd, (struct sockaddr*)& addr, sizeof(addr)) == -1){perror("bind"); exit(EXIT_FAILURE);}
    if(listen(server_fd, 5) == -1){perror("listen"); exit(EXIT_FAILURE);}
    printf("服务器启动，等待连接: %s\n", SOCKET_PATH);
    // 5.循环接受连接，并进行读写的回显
    while(1){
        int client_fd = accept(server_fd, NULL, NULL);
        if(client_fd == -1){perror("accept"); continue;}
        printf("客户端已连接\n");

        char buf[BUF_SIZE];
        ssize_t n;
        while((n = read(client_fd, buf, BUF_SIZE - 1)) > 0){
            buf[n] = '\0';
            printf("收到：%s\n", buf);
            // 服务器逻辑：将接收到的字符转为大写
            for(int i = 0; i < n; i++){
                if(buf[i] >= 'a' && buf[i] <= 'z'){buf[i] -= 32;}
            }
            write(client_fd, buf, n);
        }
        close(client_fd);
        printf("客户端已断开\n");
    }
    close(server_fd);
    unlink(SOCKET_PATH);
    printf("服务器已断开\n");
    return 0;
}