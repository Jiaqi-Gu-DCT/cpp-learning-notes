#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include "packet.h"

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1){perror("socket"); return EXIT_FAILURE;}

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){perror("inet_pton"); return EXIT_FAILURE;}

    if(connect(fd, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr)) == -1){
        perror("connect"); return EXIT_FAILURE;
    }

    printf("已连接到服务器，输入quit以退出\n");

    std::string line;
    while(std::getline(std::cin, line)){
        if(line == "quit") break;

        // 封包发送
        if(!send_packet(fd, line)){perror("send_packet"); break;}

        // 接收完整的转大写包
        std::string reply;
        if(!recv_packet(fd, reply)){perror("recv_packet"); break;}
        printf("接收到回显内容：%s\n", reply.c_str());
    }
    close(fd);
    return 0;
}