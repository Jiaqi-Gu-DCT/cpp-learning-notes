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
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd == -1) {perror("socket"); exit(EXIT_FAILURE);}
    // 2.配置地址，发起连接
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    if(connect(sockfd, (struct sockaddr*)& addr, sizeof(addr)) == -1){perror("connect"); exit(EXIT_FAILURE);}
    printf("已连接到服务器\n");
    // 3.发送数据与接收回复
    const char *msg = "from client: this is Munro!";
    write(sockfd, msg, strlen(msg));
    char buf[BUF_SIZE];
    ssize_t n = read(sockfd, buf, BUF_SIZE - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("服务器回显: %s\n", buf);
    }
    close(sockfd);
    return 0;
}