#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include "net_util.h"

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1){perror("socket"); return EXIT_FAILURE;}

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    if(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0){
        perror("inet_pton");
        return EXIT_FAILURE;
    }

    if(connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1){
        perror("connect");
        return EXIT_FAILURE;
    }
    printf("已连接服务器，输入内容回车发送，输入 quit 退出\n");

    std::string line;
    while(std::getline(std::cin, line)){
        if(line == "quit") break;
        line.push_back('\n');
        if(writen(fd, line.data(), line.size()) == -1){perror("writen"); break;}

        char buf[1024];
        ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
        if(n > 0){
            buf[n] = '\0';
            printf("回显：%s\n", buf);
        }else if(n = 0){
            printf("服务器已关闭\n");
            break;
        }
    }
    close(fd);
    return 0;
}