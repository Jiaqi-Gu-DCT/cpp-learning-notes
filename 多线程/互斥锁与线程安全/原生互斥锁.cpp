// 1.创建一些线程，每个线程对一个全局变量累加一些次数。观察加锁与不加锁前后输出结果
// 2.本题应使用原生互斥锁，并采取静态初始化宏来初始化
// 把锁加在 for 循环外面（整段只加一次锁）性能会高很多，
// 但会变成完全串行，失去多线程意义；工程中要在「线程安全」和「并发性能」之间平衡。
#include <iostream>
#include <pthread.h>
using namespace std;

const int THREAD_NUM = 4;
const int TIMES = 10000;

int p_num = 0;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void* loop(void* args){
    for(int i = 0; i < TIMES; i++){
        pthread_mutex_lock(&mtx);
        p_num ++;
        pthread_mutex_unlock(&mtx);
    }
    return nullptr;
} 

int main(){
    pthread_t tids[THREAD_NUM];
    for(int i = 0; i < THREAD_NUM; i++){
        pthread_create(&tids[i], nullptr, loop, nullptr);
    }
    for(int i = 0; i < THREAD_NUM; i++){
        pthread_join(tids[i], nullptr);
    }
    cout << "最终结果为：" << p_num << endl;
    pthread_mutex_destroy(&mtx);
    return 0;
}

