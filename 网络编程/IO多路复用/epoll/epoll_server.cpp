// epoll 服务器：监听fd LT模式 + 连接fd ET模式 + 非阻塞IO + 转大写回显
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cerrno>

const int MAX_EVENTS = 1024;

static void set_nonblock(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(){
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1){perror("socket"); return EXIT_FAILURE;}

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(8080);

    if(bind(listen_fd, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr)) == -1){
        perror("bind"); return EXIT_FAILURE;
    }
    if(listen(listen_fd, 128) == -1){perror("listen"); return EXIT_FAILURE;}

    // 创建一个epoll实例
    int epfd = epoll_create1(0);
    if(epfd == -1){perror("epoll_create1"); return EXIT_FAILURE;}

    epoll_event listen_event{};
    listen_event.data.fd = listen_fd;
    listen_event.events = EPOLLIN; // 默认是水平触发LT
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &listen_event);

    // 存放就绪连接
    epoll_event events[MAX_EVENTS];
    printf("epoll服务器启动，监听端口8080\n");
    while(true){
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if(n < 0){
            if(errno == EINTR) continue;
            perror("epoll_wait"); continue;}
        for(int i = 0; i < n; i++){
            int fd = events[i].data.fd;
            if(fd == listen_fd){// 第一分支：新的连接
                sockaddr_in cli_addr{};
                socklen_t addrlen = sizeof(cli_addr);
                int conn_fd = accept(fd, reinterpret_cast<sockaddr*>(&cli_addr), &addrlen);
                if(conn_fd == -1){perror("accept"); continue;}

                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &cli_addr.sin_addr, ip, sizeof(ip));
                printf("新-客户端：%s:%d已连接\n", ip, ntohs(cli_addr.sin_port));

                // 新的业务连接fd应该设置为非阻塞（ET）
                set_nonblock(conn_fd);

                // 以ET边沿触发模式加入epoll
                epoll_event cli_event{};
                cli_event.events = EPOLLIN | EPOLLET;
                cli_event.data.fd = conn_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, conn_fd, &cli_event);
            }else{
                char buf[1024];
                bool close_conn = false;

                // ET触发的第二个条件：循环读 直到返回EAGAIN
                while(true){
                    ssize_t r = recv(fd, buf, sizeof(buf), 0);
                    if(r > 0){
                        // 客户端有新的消息
                        for(size_t j = 0; j < r; j++){
                            if(buf[j] >= 'a' && buf[j] <= 'z') buf[j] -= 32;
                        }
                        if(send(fd, buf, static_cast<size_t>(r), 0) == -1){perror("send"); close_conn = true; break;}
                    }else if(r == 0){
                        // 客户端正常关闭连接
                        printf("fd = %d断开连接\n", fd);
                        close_conn = true;
                        break;
                    }else{
                        // 判断错误类型
                        if(errno == EAGAIN || errno == EWOULDBLOCK) {break; // 缓冲区全部读完
                        } else if(errno == EINTR) {continue; // 信号中断
                            }else{ // 真正的读错误
                                perror("recv");  close_conn = true; break;
                            }
                    }
                }
                if(close_conn){
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
                    close(fd);
                }
            }
        }
    }
    close(listen_fd);
    close(epfd);
    return 0;
}