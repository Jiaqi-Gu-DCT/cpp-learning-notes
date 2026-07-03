// 题目要求：经典的死锁场景如两个线程反复申请两把锁，程序会永久卡死；
// 而adopt_lock的搭配将修复之，保证不会思索，理解其修复原理。
// 用C++11标准实现。
#include <iostream>
#include <thread>
#include <mutex>
using namespace std;

mutex m1, m2;

void funcA(){
    // 内部自己定义两把锁的加锁顺序
    lock(m1, m2);
    // adopt_lock的作用是告知当前锁守卫，已经加过锁，无需再加锁，仅获取掌控权
    lock_guard<mutex> lg1(m1, adopt_lock);
    lock_guard<mutex> lg2(m2, adopt_lock);
    cout << "线程A已拿到两把锁" << endl;
}

void funcB(){
    lock(m1, m2);
    lock_guard<mutex> lg1(m1, adopt_lock);
    lock_guard<mutex> lg2(m2, adopt_lock);
    cout << "线程B已拿到两把锁" << endl;
}

int main(){
    thread tA(funcA), tB(funcB);
    tA.join();
    tB.join();
    cout << "程序正常结束" << endl;
    return 0;
}

// 自己写一个死锁的例子：
// #include <iostream>
// #include <mutex>
// #include <thread>
// #include <chrono>
// using namespace std;

// mutex m1, m2;

// void funcA(){
//     m1.lock();
//     cout << "A拿到m1 准备拿m2" << endl;
//     this_thread::sleep_for(chrono::milliseconds(10));
//     m2.lock();
//     cout << "A拿到了两把锁" << endl;
//     m2.unlock();
//     m1.unlock();
// }

// void funcB(){
//     m2.lock();
//     cout << "B拿到m2 准备拿m1" << endl;
//     this_thread::sleep_for(chrono::milliseconds(10));
//     m1.lock();
//     cout << "B拿到了两把锁" << endl;
//     m1.unlock();
//     m2.unlock();
// }

// int main(){
//     thread tA(funcA), tB(funcB);
//     tA.join();
//     tB.join();
//     cout << "程序正常结束" << endl;
//     return 0;
// }