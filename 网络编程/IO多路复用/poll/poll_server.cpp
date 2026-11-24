// poll 版单线程转大写回显服务器
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <vector>

int main(){
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1){perror("listen_fd"); return EXIT_FAILURE;}

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listen_fd, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr)) == -1){perror("bind"); return EXIT_FAILURE;}
    if(listen(listen_fd, 128) == -1){perror("listen"); return EXIT_FAILURE;}

    std::vector<pollfd> fds;
    // 将监听fd加入数组
    pollfd listen_pfd{};
    listen_pfd.fd = listen_fd;
    listen_pfd.events = POLLIN; // 关注【可读事件】：新的连接到达
    fds.push_back(listen_pfd);
    printf("poll回显服务器已启动，监听端口8080\n");

    while(true){
        // 参数：数组首地址，元素个数，等待时间
        int n = poll(fds.data(), fds.size(), -1);
        if(n < 0){
            if(errno == EINTR) continue;
            perror("poll"); continue;
        }
        for(size_t i = 0; i < fds.size(); i++){
            // 没有可读事件则跳过
            if(!(fds[i].revents & POLLIN)) continue;
            // 分支一：当前就绪的是监听fd，代表有新的连接
            if(fds[i].fd == listen_fd){
                sockaddr_in addr{};
                socklen_t addr_len = sizeof(addr);
                int conn_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&addr), &addr_len);
                if(conn_fd == -1){perror("accept"); continue;}

                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
                printf("新-客户端：%s:%d连接\n", ip, ntohs(addr.sin_port));

                // 把新的连接加入数组
                pollfd cli_pfd{};
                cli_pfd.fd = conn_fd;
                cli_pfd.events = POLLIN;
                fds.push_back(cli_pfd);
            }else { // 分支二：当前就绪的是普通fd
                char buf[1024];
                ssize_t r = recv(fds[i].fd, buf, sizeof(buf), 0);
                if(r <= 0){
                    // 但是客户端已经断开连接
                    printf("客户端%d断开连接\n", fds[i].fd);
                    close(fds[i].fd);

                    fds[i] = fds.back();
                    fds.pop_back();
                    i--;
                }else{
                    // 客户端发来了新的数据，需要处理
                    for(ssize_t j = 0; j < r; j++){
                        if(buf[j] >= 'a' && buf[j] <= 'z') buf[j] -= 32;
                    }
                    // 回显
                    if(send(fds[i].fd, buf, r, 0) == -1){
                        perror("send"); continue;
                    }
                }
            }
        }
    }
    close(listen_fd);
    return 0;
}