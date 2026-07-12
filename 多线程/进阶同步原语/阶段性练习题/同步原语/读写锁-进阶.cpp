// 针对 读写锁-线程安全配置管理器.cpp这个文件进行追加练习：
// 需求更改：要求读线程必须读到写线程写入的最新数据才能退出
#include <iostream>
#include <chrono>
#include <fstream>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <shared_mutex>
using namespace std;
const string FILE_DIR = "/home/sgaeyoco/桌面/cpp/多线程/进阶同步原语/阶段性练习题/同步原语/config.txt";

class Config{
friend void ReadTask(Config& cfg);
private:
    shared_mutex rwlock;
    atomic<int> write_version{0};
public:
    void readFile(int& out_version){
        shared_lock<shared_mutex> sl(rwlock);
        ifstream in(FILE_DIR, ios::in);
        if(!in.is_open()){
            out_version = write_version.load();
            return;
        }
        int count = 0;
        string buf;
        while(getline(in, buf)){
            count++;
        }
        cout << "【读线程" << this_thread::get_id() << "】读取到" << count << "行数据。" << endl;
        out_version = write_version.load();
    }

    void writeFile(const string& data){
        unique_lock<shared_mutex> ul(rwlock);
        ofstream out(FILE_DIR, ios::app);
        if(out.is_open()){
            out << data << endl;
            cout << "【写线程】写入：" << data << endl;
            write_version.fetch_add(1);
        }
    }
};

void ReadTask(Config& cfg){
    int last_read_version = -1;
    while(true){
        int current_version;
        cfg.readFile(current_version);
        last_read_version = current_version;

        if(last_read_version == cfg.write_version.load()){
            break;
        }

        this_thread::sleep_for(chrono::milliseconds(50));
    }
    cout << "【读线程" << this_thread::get_id() << "】已经读到最新数据，退出！" << endl;

}

void WriteTask(Config& cfg){
    for(int i = 0; i < 10; i++){
        this_thread::sleep_for(chrono::milliseconds(200));
        auto now = chrono::system_clock::now();
        auto ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
        string data = "更新#" + to_string(i) + "时间戳=" + to_string(ms);
        cfg.writeFile(data);
    }
    cout << "写线程" << this_thread::get_id() << "写完10轮数据！" << endl;
}

int main(){
    Config cfg;
    vector<thread> reader;
    for(int i = 0; i < 5; i++){
        reader.emplace_back(ReadTask, std::ref(cfg));
    }
    thread writer(WriteTask, std::ref(cfg));
    writer.join();
    for(auto& th : reader){
        th.join();
    }
    cout << "任务完成！" << endl;
    return 0;
}