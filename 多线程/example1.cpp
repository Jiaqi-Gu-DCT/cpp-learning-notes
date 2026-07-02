#include<iostream>
#include <thread>
#include <mutex>
#include <chrono>
using namespace std;


int main(){
    mutex mtx;
    auto task = [&mtx](int id){
        for(int i = 0; i < 3; i++){
            lock_guard<mutex> lg(mtx);
            cout << "线程" << id << "正在进行第" << i+1 << "次打印任务。" << endl;
            this_thread::sleep_for(chrono::milliseconds(1));
        }
    };
    thread t1(task,1);
    thread t2(task,2);
    thread t3(task,3);

    t1.join();
    t2.join();
    t3.join();

    cout << "所有打印任务均已完成！" << endl;
    return 0;

}