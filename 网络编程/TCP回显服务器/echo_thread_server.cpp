#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <cstring>
#include "net_util.h"

void handle_function(int conn_fd){
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

int main(){
    std::signal(SIGPIPE, SIG_IGN);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1){perror("socket"); return EXIT_FAILURE;}

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);
    if(bind(listen_fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) == -1){
        perror("bind");
        return EXIT_FAILURE;
    }

    if(listen(listen_fd, 128) == -1){
        perror("listen");
        return EXIT_FAILURE;
    }
    std::cout << "服务器启动，监听端口 8080..." << std::endl;

    while(true){
        sockaddr_in cli_addr{};
        socklen_t cli_len = sizeof(cli_addr);
        int conn_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&cli_addr), &cli_len);
        if(conn_fd == -1){perror("accept"); continue;}

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli_addr.sin_addr, ip, sizeof(ip));
        printf("客户端接入：%s:%d\n", ip, ntohs(cli_addr.sin_port));

        std::thread(handle_function, conn_fd).detach();
    }
    close(listen_fd);
    return 0;
}