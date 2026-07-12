// 实现一个配置管理类，支持读取配置项、修改配置项：
//     1.高频读，低频写，使用读写锁保证线程安全；
//     2.启动5个读线程循环读取配置，1个写线程每个200ms修改一次配置，验证并发安全。（-std=C++17）
#include <iostream>
#include <shared_mutex>
#include <mutex>
#include <chrono>
#include <thread>
#include <vector>
#include <fstream>
using namespace std;
const auto FILE_DIR = "/home/sgaeyoco/桌面/cpp/多线程/进阶同步原语/阶段性练习题/同步原语/config.txt";

class Config{
private:
    shared_mutex rwlock;
public:
    void readFile(){
        shared_lock<shared_mutex> sl(rwlock);
        ifstream in(FILE_DIR, ios::in);
        string buf;
        int count = 0;
        while(getline(in, buf)){
            count++;
        }
        cout << "[读线程" << this_thread::get_id() << "]读到" << count << "行数据。" << endl;
    }

    void writeFile(const string& data){
        unique_lock<shared_mutex> ul(rwlock);
        ofstream out(FILE_DIR, ios::app);
        out << data << endl;
        cout << "[写线程]写入：" << data << endl;
    }
};

void ReadTask(Config& cfg, int id){
    for(int i = 0; i < 20; i++){
        cfg.readFile();
        this_thread::sleep_for(chrono::milliseconds(50));
    }
    cout << "【读线程】" << id << "跑完20轮，退出！" << endl;
}

void WriteTask(Config& cfg){
    for(int i = 0; i < 10; i++){
        this_thread::sleep_for(chrono::milliseconds(200));
        auto now = chrono::system_clock::now();
        auto ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
        string data = "更新#" + to_string(i) + "时间戳=" + to_string(ms);

        cfg.writeFile(data);
    }
    cout << "【写线程】跑完10轮，退出！" << endl;
}

int main(){
    Config cfg;
    vector<thread> readers;
    for(int i = 0; i < 5; i++){
        readers.emplace_back(ReadTask, ref(cfg), i);
    }
    thread writer(WriteTask, ref(cfg));

    writer.join();
    for(auto& th : readers){
        th.join();
    }
    cout << "程序执行完毕！" << endl;
    return 0;
}