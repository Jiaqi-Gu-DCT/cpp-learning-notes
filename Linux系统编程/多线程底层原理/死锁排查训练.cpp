#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
using namespace std;

mutex m1, m2;

void funcA(){
    m1.lock();
    cout << "A拿到m1 准备拿m2" << endl;
    this_thread::sleep_for(chrono::milliseconds(10));
    m2.lock();
    cout << "A拿到了两把锁" << endl;
    m2.unlock();
    m1.unlock();
}

void funcB(){
    m2.lock();
    cout << "B拿到m2 准备拿m1" << endl;
    this_thread::sleep_for(chrono::milliseconds(10));
    m1.lock();
    cout << "B拿到了两把锁" << endl;
    m1.unlock();
    m2.unlock();
}

int main(){
    thread tA(funcA), tB(funcB);
    tA.join();
    tB.join();
    cout << "程序正常结束" << endl;
    return 0;
}