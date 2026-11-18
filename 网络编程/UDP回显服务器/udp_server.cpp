#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(){
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd == -1){perror("socket"); return EXIT_FAILURE;}

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);

    if((bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr))) == -1){
        perror("bind");
        return EXIT_FAILURE;
    }

    printf("UDP 服务器启动，监听端口 8080.\n");
    char buf[1024];
    while(true){
        sockaddr_in cli_addr{};
        socklen_t addrlen = sizeof(cli_addr);

        ssize_t n = recvfrom(fd, buf, sizeof(buf) - 1, 0, reinterpret_cast<sockaddr*>(&cli_addr), &addrlen);
        if(n == -1){perror("recvfrom"); continue;}

        buf[n] = '\0';
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli_addr.sin_addr, ip, sizeof(ip));
        printf("收到 %s:%d: %.*s\n", ip, ntohs(cli_addr.sin_port), static_cast<int>(n), buf);

        for(ssize_t i = 0; i < sizeof(buf); i++){
            if(buf[i] >= 'a' && buf[i] <= 'z'){
                buf[i] -= 32;
            }
        }

        if(sendto(fd, buf, sizeof(buf), 0, reinterpret_cast<sockaddr*>(&cli_addr), addrlen) == -1){
            perror("sendto");
        }
    }
    close(fd);
    return 0;
}