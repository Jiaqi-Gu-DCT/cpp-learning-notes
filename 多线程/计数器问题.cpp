#include <iostream>
#include <pthread.h>
#include <mutex>
using namespace std;

int g_count = 0;
pthread_mutex_t gmtx;

void* add(void* arg){
    for(int i = 0; i < 1000000; i++){
        pthread_mutex_lock(&gmtx);
        g_count++;
        pthread_mutex_unlock(&gmtx);
    }
    return nullptr;
}

int main(){
    pthread_mutex_init(&gmtx, nullptr);
    pthread_t t1, t2;
    pthread_create(&t1, nullptr, add, nullptr);
    pthread_create(&t2, nullptr, add, nullptr);
    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);
    cout << "累加后的值为：" << g_count << endl;
    pthread_mutex_destroy(&gmtx);
    return 0;
}