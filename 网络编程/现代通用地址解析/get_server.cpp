#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <unistd.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include "/home/sgaeyoco/桌面/cpp/网络编程/TCP回显服务器/net_util.h"

int main(){
    std::signal(SIGPIPE, SIG_IGN);

    // 1.定义地址过滤条件
    struct addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo* res = nullptr;

    int tmp = getaddrinfo(nullptr, "8080", &hints, &res);
    if(tmp != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(tmp));
        return EXIT_FAILURE;    
    }

    int listen_fd = -1;
    for(struct addrinfo* p = res; p != nullptr; p = p->ai_next){
        listen_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(listen_fd == -1) continue;

        int opt = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        if(bind(listen_fd, p->ai_addr, p->ai_addrlen) == 0) break;
        close(listen_fd);
        listen_fd = -1;
    }
    freeaddrinfo(res);
    if(listen_fd == -1){
        perror("bind");
        return EXIT_FAILURE;
    }

    if(listen(listen_fd, 128) == -1){
        perror("listen");
        return EXIT_FAILURE;
    }

    printf("服务器启动！监听端口 8080\n");


    while(true){
        sockaddr_in cli_addr{};
        socklen_t cli_len = sizeof(cli_addr);
        int conn_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&cli_addr), &cli_len);
        if(conn_fd == -1){perror("accept"); continue;}

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli_addr.sin_addr, ip, sizeof(ip));
        printf("客户端接入：%s:%d\n", ip, ntohs(cli_addr.sin_port));

        char buf[1024];
        ssize_t n;
        while((n = recv(conn_fd, buf, sizeof(buf), 0)) > 0){
            for(ssize_t i = 0; i < n; i++){
                if(buf[i] >= 'a' && buf[i] <= 'z'){
                    buf[i] -= 32;
                }
            }
            if(writen(conn_fd, buf, static_cast<size_t>(n)) == -1){
                perror("writen");
                break;
            }
        }
        if(n == 0){
            printf("客户端已断开\n");}
        close(conn_fd);
    }
    close(listen_fd);
    return 0;
}