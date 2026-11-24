// 单线程处理多个客户端请求
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <signal.h>

int main(){
    signal(SIGPIPE, SIG_IGN);
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1){perror("socket"); return EXIT_FAILURE;}

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(8080);

    if(bind(listen_fd, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr)) == -1){perror("bind"); return EXIT_FAILURE;}
    if(listen(listen_fd, 128) == -1){perror("listen"); return EXIT_FAILURE;}

    fd_set all_fds;
    FD_ZERO(&all_fds);
    FD_SET(listen_fd, &all_fds);
    int max_fd = listen_fd;
    printf("服务器已启动，监听端口8080\n");

    while(true){
        fd_set read_fds = all_fds;
        int n = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr); // n>0：就绪的fd数量
        if(n < 0){perror("select"); continue;}

        for(int fd = 0; fd <= max_fd; fd++){
            if(!FD_ISSET(fd, &read_fds)) continue;

            // 第一分支：就绪的是监听fd，说明是新的连接，需要accept
            if(fd == listen_fd){
                sockaddr_in addr{};
                socklen_t addr_len = sizeof(addr);
                int conn_fd = accept(fd, reinterpret_cast<sockaddr*>(&addr), &addr_len);
                if(conn_fd == -1){perror("accept"); continue;}
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
                FD_SET(conn_fd, &all_fds);
                if(conn_fd > max_fd) max_fd = conn_fd;
                printf("新-客户端接入：%s:%d\n", ip, ntohs(addr.sin_port));
            }else{ 
                char buf[1024];
                ssize_t r = recv(fd, buf, sizeof(buf), 0);
                if(r <= 0){// 第二分支：就绪的是普通fd，客户端断开连接
                    close(fd);
                    FD_CLR(fd, &all_fds);
                    printf("fd=%d断开连接\n", fd);
                }else{// 第三分支：就绪的是普通fd，客户端发来了消息，进行小写转大写处理
                    for(ssize_t i = 0; i < r; i++){
                        if(buf[i] >= 'a' && buf[i] <= 'z') buf[i] -= 32;
                    }
                    if(send(fd, buf, r, 0) == -1){perror("send"); close(fd); FD_CLR(fd, &all_fds);}
                }
            }
        }
    }
    close(listen_fd);
    return 0;
}