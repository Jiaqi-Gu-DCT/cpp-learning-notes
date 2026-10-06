#include <iostream>
#include <pthread.h>

using namespace std;

void* thread_func(void* arg){
    int num = *(int*) arg;
    cout << "子线程，参数为：" << num << "，线程ID为：" << pthread_self() << endl;
    return nullptr;
}
int main(){
    pthread_t tid;
    int value = 2026;
    int ret = pthread_create(&tid, nullptr, thread_func, &value);
    if(ret != 0){
        cout << "子线程创建失败，错误码为："<< ret << endl;
        return -1;
    }
    cout << "主线程，子线程ID=" << tid << endl;
    pthread_join(tid, nullptr);
    return 0;

}
