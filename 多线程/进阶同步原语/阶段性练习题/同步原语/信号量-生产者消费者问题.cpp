// 使用 C++20 std::counting_semaphore 实现有界缓冲区生产者-消费者模型：
//     缓冲区容量为4；
//     1个生产者线程，依次放入1~8的整数；
//     2个消费者线程，依次取出数据并打印；
//     不能使用条件变量，仅用信号量+互斥锁实现。
#include <iostream>
#include <semaphore>
#include <mutex>
#include <queue>
#include <thread>
using namespace std;

counting_semaphore<4> empty_sem(4);
counting_semaphore<4> full_sem(0);
mutex mtx;
queue<int> q;

void q_push(int val){
    empty_sem.acquire();
    lock_guard<mutex> lg(mtx);
    q.push(val);
    cout << "生产者" << this_thread::get_id() << "生产了一个数据" << val << endl;
    full_sem.release();
}

void q_pop(){
    full_sem.acquire();
    lock_guard<mutex> lg(mtx);
    int item = q.front();
    q.pop();
    cout << "消费者" << this_thread::get_id() << "消费了一个数据" << item << endl;
    empty_sem.release();
}

void produce(){
    for(int i = 1; i <= 8; i++){
        q_push(i);
    }
}

void consume(){
    for(int i = 0; i < 4; i++){
        q_pop();
    }
}

int main(){
    thread producer(produce);
    thread consumer1(consume);
    thread consumer2(consume);

    producer.join();
    consumer1.join();
    consumer2.join();

    cout << "任务完成！" << endl;
    return 0;
}