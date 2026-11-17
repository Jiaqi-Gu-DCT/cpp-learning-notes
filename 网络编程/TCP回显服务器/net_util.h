#ifndef NET_UTIL_H
#define NET_UTIL_H

#include <sys/socket.h>
#include <cstddef>
#include <cerrno>

// 可靠写
// buf指向的是要通过这个socket发送出去的数据内容，更准确地说，它是「待发送数据缓冲区的起始内存地址」
inline ssize_t writen(int fd, const void* buf, size_t count){
    size_t left = count;
    const char* ptr = static_cast<const char*>(buf);
    while(left > 0){
        ssize_t n = send(fd, ptr, left, 0);
        if(n > 0){
            left -= static_cast<size_t>(n);
            ptr += n;
        }else if(n == -1 && errno == EINTR){
            continue;;
        }else{
            return -1;
        }
    }
    return static_cast<ssize_t>(count);
}

// 可靠读
inline ssize_t readn(int fd, void* buf, size_t count){
    size_t left = count;
    char* ptr = static_cast<char*>(buf);
    while(left > 0){
        ssize_t n = recv(fd, ptr, left, 0);
        if(n > 0){
            left -= static_cast<size_t>(n);
            ptr += n;
        }else if(n == -1 && errno == EINTR){
            continue;
        }else{
            return -1;
        }
    }
    return static_cast<ssize_t>(count - left);
}

#endif