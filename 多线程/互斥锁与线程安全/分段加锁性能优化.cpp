// 题目要求：有一个共享计数器和一段耗时的计算逻辑，要求：
// 使用unique_lock的defer_lock延迟加锁方式，只在读写共享变量时加锁，耗时部分
// 提前解锁来提高并发度
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
using namespace std;

mutex mtx;
int t_count = 0;
void heavyCalc(){
    this_thread::sleep_for(chrono::microseconds(25));
}

// 全程加锁 线程的入口函数之一
void badWork(int times){
    for(int i = 0; i < times; i++){
        unique_lock<mutex> ul(mtx);
        t_count++;
        heavyCalc();
    }    
}

// 分段加锁，线程入口函数之一
void goodWork(int times){
    for(int i = 0; i < times; i++){
        unique_lock<mutex> ul(mtx, defer_lock);
        ul.lock();
        t_count++;
        ul.unlock();
        heavyCalc();
    }
}

int main(){
    const int TIMES = 1000;
    const int THREAD_NUM = 100;
    vector<thread> threads;
    
    // 全局加锁
    t_count = 0;
    auto start = chrono::steady_clock::now();
    for(int i = 0; i < THREAD_NUM; i++){
        threads.emplace_back(badWork, TIMES);
    }
    for(int i = 0; i < THREAD_NUM; i++){
        threads[i].join();
    }
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "全局加锁用时：" << duration.count() << "毫秒， 计数器的值为" << t_count << endl;
    
    // 分段加锁
    threads.clear();
    t_count = 0;
    start = chrono::steady_clock::now();
    for(int i = 0; i < THREAD_NUM; i++){
        threads.emplace_back(goodWork, TIMES);
    }
    for(int i = 0; i < THREAD_NUM; i++){
        threads[i].join();
    }
    end = chrono::steady_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "分段加锁用时：" << duration.count() << "毫秒， 计数器的值为" << t_count << endl;
    return 0;
}   