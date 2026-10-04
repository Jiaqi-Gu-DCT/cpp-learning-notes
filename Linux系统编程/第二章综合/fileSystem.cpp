// 实现一个函数 with_file_stdout：
// 1.传入文件路径和一个回调函数
// 2.原子地将标准输出（fd=1）重定向到该文件
// 3.执行回调函数（回调里的所有 printf 都会输出到文件）
// 4.回调执行完后，调用 fsync 强制落盘
// 5.恢复标准输出到终端
// 6.全程异常安全：任何步骤失败都能正确恢复标准输出、不泄漏 fd
#include "FileDescriptor.h"
#include <iostream>
#include <functional>
#include <cstdio>

void with_file_stdout(const std::string& path, std::function<void()> callback){
    // 保存原来的标准输出
    int stdout_save = ::dup(STDOUT_FILENO);
    if(stdout_save == -1){
        throw std::system_error(errno, std::generic_category(), "dup stdout");
    }

    try{
        // 打开目标文件
        FileDescriptor file(path.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0644);
        file.dup2(STDOUT_FILENO);
        callback();
        file.fsync();
    }catch(...){
        // 异常时先恢复标准输出，再重抛
        ::dup2(stdout_save, STDOUT_FILENO);
        ::close(stdout_save);
        throw;
    }

    // 正常恢复标准输出
    ::dup2(stdout_save, STDOUT_FILENO);
    ::close(stdout_save);
}

int main(){
    try {
        std::cout << "这行在终端1\n";

        with_file_stdout("output.txt", []() {
            std::cout << "这行写入文件\n";
            std::cout << "所有标准输出都被重定向了\n";
        });

        std::cout << "这行在终端2\n";
        std::cout << "标准输出已恢复\n";
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}