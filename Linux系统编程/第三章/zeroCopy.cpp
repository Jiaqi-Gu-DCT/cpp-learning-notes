#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <string.h>
#include <chrono>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <system_error>
#include <array>
#include <iostream>

class FileDescriptor{
public:

    // 构造与析构
    FileDescriptor(const char* path, int flags, mode_t mode = 0644){
        m_fd = ::open(path, flags, mode);
        if(m_fd == -1){throw std::system_error(errno, std::generic_category(), "open " + std::string(path) + " failed");}
    }
    explicit FileDescriptor(int fd)noexcept:m_fd(fd){};
    ~FileDescriptor(){reset();}

    // 禁止拷贝
    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator= (const FileDescriptor&) = delete;

    // 移动语义
    FileDescriptor(FileDescriptor&& other)noexcept:m_fd(other.m_fd){
        other.m_fd = -1;
    }
    FileDescriptor& operator=(FileDescriptor&& other)noexcept{
        // 比较地址
        if(this != &other){
            this->reset();
            this->m_fd = other.m_fd;
            other.m_fd = -1;
        }
        return *this;
    }

    void reset()noexcept{
        if(m_fd != -1){
            ::close(m_fd);
            m_fd = -1;
        }
    }

    int getFd()const noexcept{
        return this->m_fd;
    }

    size_t getSize()const{
        struct stat ft;
        if(::fstat(m_fd, &ft) == -1){throw std::system_error(errno, std::generic_category(), "fstat");}
        return ft.st_size;
    }
private:
    int m_fd = -1;
};

class MmapView{
public:
    MmapView(size_t length, int prot, int flags, int fd): m_addr(::mmap(NULL, length, prot, flags, fd, 0)), m_length(length){
       if(m_addr == MAP_FAILED){
        throw std::system_error(errno, std::generic_category(), "mmap");
       }
    }
    ~MmapView(){reset();}

    MmapView(const MmapView&) = delete;
    MmapView& operator= (const MmapView&) = delete;

    MmapView(MmapView&& other)noexcept:m_addr(other.m_addr), m_length(other.m_length){
        other.m_length = 0;
        other.m_addr = nullptr;
    }
    MmapView& operator= (MmapView&& other)noexcept{
        if(this != &other){
            this->reset();
            this->m_addr = other.m_addr;
            this->m_length = other.m_length;
            other.m_length = 0;
            other.m_addr = nullptr;
        }
        return *this;
    }

    void reset(){
        if(m_addr != nullptr){
            ::munmap(m_addr, m_length);
            m_addr = nullptr;
            m_length = 0;
        }
    }
    void* getAddr(){
        return this->m_addr;
    }
private:
    void* m_addr = nullptr;
    size_t m_length = 0;

};

class Pipe{
public:
    Pipe():Pipe(create_pipe()){}

    int getReadFd()const noexcept{
        return m_read_fd.getFd();
    }
    int getWriteFd()const noexcept{
        return m_write_fd.getFd();
    }

    Pipe(const Pipe&) = delete;
    Pipe& operator= (const Pipe&) = delete;

    Pipe(Pipe&& other) noexcept = default;
    Pipe& operator= (Pipe&& other) = default; 

    
    void reset(){
        this->m_read_fd.reset();
        this->m_write_fd.reset();
    }
private:
    FileDescriptor m_read_fd, m_write_fd;
    explicit Pipe(std::array<int,2>fds):m_read_fd(fds[0]), m_write_fd(fds[1]){}
    static std::array<int,2> create_pipe(){
        int pipefd[2];
        if(::pipe(pipefd) == -1){throw std::system_error(errno, std::generic_category(), "pipe");}
        return{pipefd[0], pipefd[1]};
    }
};

constexpr size_t BUF_SIZE = 4096;
int copyReadAndWrite(FileDescriptor& src, FileDescriptor& dst){
    char buf[BUF_SIZE];
    ssize_t n;
    while(true){
        n = ::read(src.getFd(), buf, BUF_SIZE);
        if(n == -1 && errno == EINTR) continue;
        if(n <= 0) break;
        ssize_t written = 0;
        while(written < n){
            ssize_t w = ::write(dst.getFd(), buf + written, n - written);
            if(w == -1){
                if(errno == EINTR) continue;
                throw std::system_error(errno, std::generic_category(), "write dst");
            }
            written += w;
        }
    }
    if(n == -1){
        throw std::system_error(errno, std::generic_category(), "read src");
    }
    return 0;
}

int copyMmap(FileDescriptor& src, FileDescriptor& dst){
    size_t size = src.getSize();
    if(::ftruncate(dst.getFd(), size) == -1){throw std::system_error(errno, std::generic_category(), "ftruncate dst");}
    MmapView Msrc(size, PROT_READ, MAP_PRIVATE, src.getFd());
    MmapView Mdst(size, PROT_WRITE, MAP_SHARED, dst.getFd());
    ::memcpy(Mdst.getAddr(), Msrc.getAddr(), size);
    ::msync(Mdst.getAddr(), size, MS_SYNC);
    return 0;
}

int copySendfile(FileDescriptor& src, FileDescriptor& dst){
    off_t offset = 0;
    size_t size = src.getSize();
    ssize_t sent = ::sendfile(dst.getFd(), src.getFd(), &offset, size);
    if(sent == -1){throw std::system_error(errno, std::generic_category(), "sendfile");}
    ::fsync(dst.getFd());
    return 0;
}

int copySplice(FileDescriptor& src, FileDescriptor& dst){
    size_t size = src.getSize();
    // 当对象使用空括号构造时，编译器会把它解析为一个返回 Pipe 类型、名为 pipes 的函数声明，而不是对象定义
    Pipe pipes;
    while(size > 0){
        size_t chunk = (size > 65536) ? 65536 : size;
        ssize_t s_in = ::splice(src.getFd(), NULL, pipes.getWriteFd(), NULL, chunk, 0);
        if(s_in == -1){throw std::system_error(errno, std::generic_category(), "splice s_in");}
        // 输入为0表示已经到达文件末尾了
        if(s_in == 0) break;
        ssize_t s_out = ::splice(pipes.getReadFd(), NULL, dst.getFd(), NULL, s_in, 0);
        if(s_out == -1){throw std::system_error(errno, std::generic_category(), "splice s_out");}
        // 管道内无数据 异常终止
        if(s_out == 0) break;
        size -= s_out;
    }
    ::fsync(dst.getFd());
    return 0;
}

template<typename T>
double measure_ms(T&& t){
    auto t1 = std::chrono::high_resolution_clock::now();
    t();
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    return duration.count();
}

int main(int argc, char* argv[]){
    if(argc != 3){
        fprintf(stderr, "Usage: %s <srcfile> <dst_prefix>\n", argv[0]);
        fprintf(stderr, "Creates <dst_prefix>.rw, <dst_prefix>.mmap, <dst_prefix>.sendfile, <dst_prefix>.splice\n");
        return 1;
    }
    const char* src_path = argv[1];
    const char* dst_prefix = argv[2];

    try{
        {
            std::string dst_path = std::string(dst_prefix) + ".rw";
            FileDescriptor src(src_path, O_RDONLY);
            FileDescriptor dst(dst_path.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0644);
            double ms_rw = measure_ms([&](){copyReadAndWrite(src, dst);});
            std::cout << "Read and write 耗时：" << ms_rw << " ms。" << std::endl;
        }

        {
            std::string dst_path = std::string(dst_prefix) + ".mmap";
            FileDescriptor src(src_path, O_RDONLY);
            FileDescriptor dst(dst_path.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0644);
            double ms_mmap = measure_ms([&](){copyMmap(src, dst);});
            std::cout << "Mmap 耗时：" << ms_mmap << " ms。" << std::endl;
            
        }

        {
            std::string dst_path = std::string(dst_prefix) + ".sendfile";
            FileDescriptor src(src_path, O_RDONLY);
            FileDescriptor dst(dst_path.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0644);
            double ms_sendfile = measure_ms([&](){copySendfile(src, dst);});
            std::cout << "Sendfile 耗时：" << ms_sendfile<< " ms。" << std::endl;
        }

        {
            std::string dst_path = std::string(dst_prefix) + ".splice";
            FileDescriptor src(src_path, O_RDONLY);
            FileDescriptor dst(dst_path.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0644);
            double ms_splice = measure_ms([&](){copySplice(src, dst);});
            std::cout << "Splice 耗时：" << ms_splice << " ms。" << std::endl;
        }

    }catch(const std::exception& e){
        fprintf(stderr,"测试失败： %s！\n", e.what());
        return 1;
    }
    return 0;
}