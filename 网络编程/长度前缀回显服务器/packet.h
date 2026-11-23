#ifndef PACKET_H
#define PACKET_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdint>
#include <vector>
#include <string>
#include "/home/sgaeyoco/桌面/cpp/网络编程/TCP回显服务器/net_util.h" 

// 封包
inline bool send_packet(int fd, const std::string& msg){
    // uint32_t占4字节
    uint32_t len = htonl(static_cast<uint32_t>(msg.size()));
    // 先写入4字节的长度头
    if(writen(fd, &len, sizeof(len)) == -1) return false;
    // 再写入消息体
    if(writen(fd, msg.data(), msg.size()) == -1) return false;
    return true;
}

// 简化版的拆包
inline bool recv_packet(int fd, std::string& out_msg){
    uint32_t len_net = 0;
    ssize_t n = readn(fd, &len_net, sizeof(len_net));
    if(n == 0 || n == -1) return false;
    uint32_t body_len = ntohl(len_net);
    out_msg.resize(body_len);
    n = readn(fd, out_msg.data(), body_len);
    return static_cast<uint32_t>(n) == body_len;
}

#endif