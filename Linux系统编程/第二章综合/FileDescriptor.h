#pragma once
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <utility>
#include <system_error>
#include <string>
#include <cstring>

class FileDescriptor{
    // 底层负责错误检测，调用层上层负责错误处理(try...catch...)
public:
    // 构造函数：打开文件
    FileDescriptor(const char* path, int flags, mode_t mode = 0644){
        // ::表示 令编译器在外部全局作用域中寻找
        m_fd = ::open(path, flags, mode);
        if(m_fd == -1){
            // throw:一旦执行这行，当前函数立刻终止，不会继续往下走。异常对象会沿着函数调用栈向上找，直到找到能匹配它的 catch 语句；
            // 如果全程都没捕获，程序直接终止并打印错误信息。
            throw std::system_error(errno, std::generic_category(), "open" + std::string(path) + "failed.");
        }
    }

    // 从已有的fd接管文件所有权：绝对不会在构造时出错, explicit是防止编译器隐式转换的 直接转换为需要的类型
    explicit FileDescriptor(int fd) noexcept:m_fd(fd){}

    // 析构：自动关闭
    ~FileDescriptor(){reset();}

    // 禁止拷贝（fd的唯一所有权问题）
    // [类名](const [类名]&) = delete; 拷贝构造
    // [类名]& operator=(const [类名]&) = delete; 拷贝赋值运算符
    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete; 

    // 支持移动语义:移动构造 移动赋值运算符
    FileDescriptor(FileDescriptor&& other)noexcept:m_fd(other.m_fd){
        other.m_fd = -1;
    }

    FileDescriptor& operator= (FileDescriptor&& other)noexcept{
        if(this != &other){
            reset();
            m_fd = other.m_fd;
            other.m_fd = -1;
        }
        return *this;
    }

    // 显式关闭
    void reset()noexcept{
        if(m_fd == -1){
            ::close(m_fd);
            m_fd = -1;
        }
    }

    // 获取原始的fd
    int get() const noexcept {return m_fd;}

    bool valid() const noexcept {return m_fd != -1;}
    // operator bool():当这个对象需要被当作 bool 类型使用时，该怎么做
    explicit operator bool() const noexcept {return valid();}

    // 读取文件
    ssize_t read(void* buf, size_t count) const{
        ssize_t n;
        while((n = ::read(m_fd, buf, count)) == -1 && errno == EINTR);
        return n;
    }

    // 循环写满
    ssize_t write(const void* buf, size_t count) const{
        const char* p = static_cast<const char*>(buf);
        size_t total = 0;
        while(total < count){
            ssize_t n = ::write(m_fd, p + total, count - total);
            if(n == -1){
                if(errno == EINTR) continue;
                return -1;
            }
            total += n;
        }
        return total;
    }

    // 移动指针
    off_t seek(off_t offset, int whence) const{
        return ::lseek(m_fd, offset, whence);
    }

    // 强制落盘
    void fsync() const{
        if(::fsync(m_fd) == -1){
            throw std::system_error(errno, std::generic_category(), "fsync failed");
        }
    }

    void fdatasync() const{
        if(::fdatasync(m_fd) == -1){
            throw std::system_error(errno, std::generic_category(), "fdatasync failed");
        }
    }

    // 复制fd（返回新的FileDescriptor）
    FileDescriptor dup() const{
        int new_fd = ::dup(m_fd);
        if(new_fd == -1){
            throw std::system_error(errno, std::generic_category(), "dup failed");
        }
        return FileDescriptor(new_fd);
    }

    // 复制到指定的fd
    void dup2(int target_fd) const{
        if(::dup2(m_fd, target_fd) == -1){
            throw std::system_error(errno, std::generic_category(), "dup2 failed");
        }
    }

    // 设定/取消非阻塞
    void set_nonblocking(bool enable = true) const{
        int flags = ::fcntl(m_fd, F_GETFL);
        if(flags == -1){
            throw std::system_error(errno, std::generic_category(), "fcntl F_GETFL");
        }
        if(enable) flags |= O_NONBLOCK;
        else flags &= ~O_NONBLOCK;
        if(::fcntl(m_fd, F_SETFL, flags) == -1){
            throw std::system_error(errno, std::generic_category(), "fcntl F_SETFL");
        }

    }

    // 设置 FD_CLOEXEC
    void set_cloexec(bool enable = true) const{
        int flags = ::fcntl(m_fd, F_GETFD);
        if(flags == -1){
            throw std::system_error(errno, std::generic_category(), "fcntl F_GETFD");
        }
        if(enable) flags |= FD_CLOEXEC;
        else flags &= ~FD_CLOEXEC;
        if(::fcntl(m_fd, F_SETFD, flags) == -1){
            throw std::system_error(errno, std::generic_category(), "fcntl F_SETFD");
        }
    }

    // 截断文件
    void truncate(off_t length) const{
        if(::ftruncate(m_fd, length) == -1){
            throw std::system_error(errno, std::generic_category(), "ftruncate");
        }
    }
private:
    int m_fd = -1;
};