// 单线程处理多个客户端请求
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1){perror("socket"); return EXIT_FAILURE;}

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    if(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0){perror("inet_pton"); return EXIT_FAILURE;}

    if(connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1){
        perror("connect"); 
        return EXIT_FAILURE;
    }
    printf("客户端连接到服务， 输入quit以断开连接\n");

    std::string line;
    while(std::getline(std::cin, line)){
        if(line == "quit") break;
        line.push_back('\n');

        if(write(fd, line.data(), line.size()) == -1){perror("write"); break;}

        char buf[1024];
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if(n > 0){
            buf[n] = '\0';
            printf("服务器回显：%s\n", buf);
        }else if(n == 0){
            printf("服务器已关闭\n");
            break;
        }
    }

    close(fd);
    return 0;

}