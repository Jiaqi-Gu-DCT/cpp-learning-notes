// 题目要求：练习设置锁的属性为递归锁后验证同一线程多次加锁问题，
//最好打印一下加解锁的次数（限定采用原生线程方式）
#include <iostream>
#include <pthread.h>
using namespace std;

pthread_mutex_t mtx;

void funcInner(){
    pthread_mutex_lock(&mtx);
    cout << "进入内层函数，加锁" << endl;
    pthread_mutex_unlock(&mtx);
    cout << "退出内层函数，解锁" << endl;
}

void funcOutter(){
    pthread_mutex_lock(&mtx);
    cout << "进入外层函数，加锁" << endl;
    funcInner();
    pthread_mutex_unlock(&mtx);
    cout << "退出外层函数，解锁" << endl;
}

int main(){
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mtx, &attr);
    pthread_mutexattr_destroy(&attr);

    funcOutter();

    pthread_mutex_destroy(&mtx);
    return 0;
}
// 拓展 ：在下面采用C++11 std::mutex 的方式进行编写
// #include <iostream>
// #include <mutex>
// using namespace std;

// recursive_mutex rmtx;

// void funcInner(){
//     lock_guard<recursive_mutex> lg(rmtx);
//     cout << "进入内层函数，加锁" << endl;
// }

// void funcOutter(){
//     lock_guard<recursive_mutex> lg(rmtx);
//     cout << "进入外层函数，加锁" << endl;
//     funcInner();
// }

// int main(){
//     funcOutter();
//     return 0;
// }