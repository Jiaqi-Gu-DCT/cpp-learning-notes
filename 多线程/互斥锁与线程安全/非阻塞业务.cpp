// 实现一个简单的日志写入功能，多个线程同时向全局日志缓存区写日志
// 加锁失败就放弃本次日志写入，计算成功/失败的写入日志条数、
#include <iostream>
#include <pthread.h>
#include <string>
#include <vector>
using namespace std;

const int THREAD_NUM = 5;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
int success = 0;
int fail = 0; 

void writeLog(const string& msg){
    if(pthread_mutex_trylock(&mtx) == 0){
        success++;
        cout << "写入日志成功！" << endl;
        cout << "【日志】" << msg << endl;
        pthread_mutex_unlock(&mtx);
    }else{
        pthread_mutex_lock(&mtx);
        fail++;
        cout << "写入日志失败！" << endl;
        pthread_mutex_unlock(&mtx);
    }
}

void* work(void* arg){
    int id = *static_cast<int*>(arg); 
    for(int i = 0; i < 100; i++){
        writeLog("线程：" + to_string(id) + "第" + to_string(i) + "条日志。");
    }
    return nullptr;
}

int main(){
    pthread_t tids[THREAD_NUM];
    vector<int> ids(THREAD_NUM);
    for(int i = 0; i < THREAD_NUM; i++){
        ids[i] = i;
        pthread_create(&tids[i], nullptr, work, &ids[i]);
    }

    for(int i = 0; i < THREAD_NUM; i++){
        pthread_join(tids[i], nullptr);
    }
    cout << "成功写入日志：" << success << "次"  << endl;
    cout << "写入日志失败：" << fail << "次"  << endl;
    pthread_mutex_destroy(&mtx);

    return 0;
}