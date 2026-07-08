#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <vector>
using namespace std;

shared_mutex rwlock;

void read(const vector<int>& s_data){
    shared_lock<shared_mutex> sl(rwlock);
    auto start = chrono::steady_clock::now();
    cout << "线程" << this_thread::get_id() << "正在读取数据" << endl;
    if(s_data.empty()){
        cout << "当前无数据" << endl;
    }else{
        for (auto i : s_data){
        cout << i << endl;
    }
    }
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "线程" << this_thread::get_id() << "读操作花费了" << duration.count() << "微秒。" << endl;
}

void write(const vector<int>& t, vector<int>& s_data){
    unique_lock<shared_mutex> sl(rwlock);
    auto start = chrono::steady_clock::now();
    cout << "线程" << this_thread::get_id() << "正在写入数据" << endl;
    s_data.insert(s_data.end(), t.begin(), t.end());
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "线程" << this_thread::get_id() << "写操作花费了" << duration.count() << "微秒。" << endl;
}

int main(){
    auto start = chrono::steady_clock::now();
    vector<int>s_data, insert_data;
    for(int i = 1; i < 100; i++){
        insert_data.push_back(i);
    }
    thread reader1(read, ref(s_data));
    thread writer1(write, ref(insert_data), ref(s_data));
    thread reader2(read, ref(s_data));
    reader1.join();
    reader2.join();
    writer1.join();
    thread reader3(read, ref(s_data));
    reader3.join();
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "程序执行完毕，耗时" << duration.count() << "微秒。" << endl;
    return 0;
}