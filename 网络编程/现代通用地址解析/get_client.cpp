#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <netdb.h>
#include <iostream>
#include <string>
#include "/home/sgaeyoco/桌面/cpp/网络编程/TCP回显服务器/net_util.h"

int main(){
    struct addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo* ref = nullptr;

    int tmp = getaddrinfo("127.0.0.1", "8080", &hints, &ref);
    if(tmp != 0){fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(tmp)); return EXIT_FAILURE;}

    int fd = -1;
    for(struct addrinfo* p = ref; p != nullptr; p = p->ai_next){
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(fd == -1) continue;

        if(connect(fd, p->ai_addr, p->ai_addrlen) == 0){
            break;
        }
        close(fd);
        fd = -1;
    }
    freeaddrinfo(ref);
    if(fd == -1){
        printf("客户端连接失败\n");
        return EXIT_FAILURE;
    }
    printf("客户端已连接， 输入 quit 断连\n");

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