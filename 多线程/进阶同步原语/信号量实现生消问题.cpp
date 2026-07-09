#include <iostream>
#include <mutex>
#include <thread>
#include <semaphore>
#include <queue>
using namespace std;

mutex mtx;
queue<int> ware;
counting_semaphore<3> empty_sem(3);
counting_semaphore<3> full_sem(0);

void producer(const int& item){
    empty_sem.acquire();
    lock_guard<mutex> lock(mtx);
    ware.push(item);
    cout << "生产者" << this_thread::get_id() << "生产了一个单位" << endl;
    full_sem.release();
}

void consumer(){
    full_sem.acquire();
    lock_guard<mutex> lock(mtx);
    int val = ware.front();
    ware.pop();
    cout << "消费者" << this_thread::get_id() << "消费了一个单位:" << val << endl;
    empty_sem.release();
}

int main(){
    thread produce1(producer, 1);
    thread produce2(producer, 2);
    thread produce3(producer, 3);
    thread consume1(consumer);
    thread consume2(consumer);

    produce1.join();
    produce2.join();
    consume1.join();
    produce3.join();
    consume2.join();

    int length = ware.size();
    cout << "当前队列长度为：" << length << endl;
    for(int i = 0; i < length; i++){
        cout << "第" << i+1 << "个值为：" << ware.front() << endl;
        ware.pop();
    }
    return 0;
}