#include<iostream>
#include<pthread.h>
using namespace std;

pthread_mutex_t mtx;
void funcB();

void funcA(){
    pthread_mutex_lock(&mtx);
    cout << "A函数调用中" << endl;
    pthread_mutex_unlock(&mtx);
    cout << "结束A函数的调用" << endl;
}

void funcB(){
    pthread_mutex_lock(&mtx);
    funcA();
    pthread_mutex_unlock(&mtx);
}

int main(){
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mtx, &attr);
    pthread_mutexattr_destroy(&attr);

    funcB();

    pthread_mutex_destroy(&mtx);
    return 0;
}