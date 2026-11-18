#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

int main(){
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd == -1){perror("socket"); return EXIT_FAILURE;}

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr) <= 0){
        perror("inet_pton");
        return EXIT_FAILURE;
    }

    printf("UDP 客户端就绪， 输入quit退出\n");
    std::string line;
    while(std::getline(std::cin, line)){
        if(line == "quit") break;

        if((sendto(fd, line.data(), line.size(), 0, reinterpret_cast<const sockaddr*>(&serv_addr), sizeof(serv_addr)) == -1)){
            perror("sendto");
            break;
        }

        char buf[1024];
        sockaddr_in recv_addr{};
        socklen_t addrlen = sizeof(recv_addr);
        ssize_t n = recvfrom(fd, buf, sizeof(buf) - 1, 0, reinterpret_cast<sockaddr*>(&recv_addr), &addrlen);
        if(n > 0){
            buf[n] = '\0';
            printf("收到回显：%s\n", buf);
        }else if(n == -1){
            perror("recvfrom");
            break;
        }
    }
    close(fd);
    return 0;
}