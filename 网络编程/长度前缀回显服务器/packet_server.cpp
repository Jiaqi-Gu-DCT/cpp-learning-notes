#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <string>
#include "packet.h"

// 单个连接的业务处理：转大写回显 + 应用层缓冲区拆包
void handle_client(int conn_fd){
    std::string recv_buf; // 每个连接独有的应用层接收缓冲区
    char tmp_buf[1024]; // 每次recv的临时读取缓冲区
    while(true){
        // 1.从内核心读取数据，读到多少算多少
        ssize_t n = recv(conn_fd, tmp_buf, sizeof(tmp_buf), 0);
        if(n > 0){
            // 2.追加到应用层缓冲区末尾
            recv_buf.append(tmp_buf, static_cast<size_t>(n));
            // 3.循环拆包：尽可能取出所有完整信息
            while(recv_buf.size() >= sizeof(uint32_t)){
                // 3.1 先取出4字节的长度头 并转为主机序
                uint32_t len_net = 0;
                std::memcpy(&len_net, recv_buf.data(), sizeof(len_net));
                uint32_t body_len = ntohl(len_net);
                // 3.2 判断缓冲区里的数据是否为一整条消息
                // 包体不完整则退出循环，等待下次recv凑齐
                if(recv_buf.size() < sizeof(uint32_t) + body_len) break;
                // 3.3 取出一则完整消息
                std::string msg(recv_buf.begin() + sizeof(uint32_t), recv_buf.begin() + sizeof(uint32_t) + body_len);
                // 3.4 业务处理逻辑：将小写字母全部转换为大写
                for(char& c : msg){
                    if(c >= 'a' && c <= 'z') c -= 32;
                }
                // 3.5 封包，回发给客户端
                send_packet(conn_fd, msg);
                // 3.6 从缓冲区头部删除已处理过的包
                recv_buf.erase(0, sizeof(uint32_t) + body_len);
            }
        }else if(n == 0){
            printf("客户端断开连接\n"); break;
        }else{
            if(errno == EINTR) continue;
            perror("recv"); break;
        }
    }
    close(conn_fd);
}

int main(){
    signal(SIGPIPE, SIG_IGN);
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1){perror("socket"); return EXIT_FAILURE;}

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1){perror("bind"); return EXIT_FAILURE;}
    if(listen(listen_fd, 128) == -1){perror("listen"); return EXIT_FAILURE;}
    printf("长度前缀回显服务器启动，监听端口8080\n");

    while(true){
        sockaddr_in cli_addr{};
        socklen_t cli_len = sizeof(cli_addr);

        int conn_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&cli_addr), &cli_len);
        if(conn_fd == -1){perror("accept"); continue;}
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli_addr.sin_addr, ip, sizeof(ip));
        printf("客户端接入：%s:%d\n", ip, ntohs(cli_addr.sin_port));

        std::thread(handle_client, conn_fd).detach();
    }
    close(listen_fd);
    return 0;
}