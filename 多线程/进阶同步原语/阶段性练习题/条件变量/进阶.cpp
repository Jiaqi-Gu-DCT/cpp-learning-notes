// 题目要求：启动 2 个生产者线程，每个线程生产 10 个整数（共20个数据）；启动 3 个消费者线程，持续从队列取数据；
// 所有数据生产完毕后，消费者线程能正常检测到结束信号并退出，不能出现永久阻塞卡死；
// 全程保证线程安全，输出不要求顺序，但不能出现重复消费、漏消费。
#include <iostream>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>
using namespace std;

class S_queue{
private:
    queue<int> q;
    int capacity;
    mutex mtx;
    condition_variable not_full, not_empty;
    bool produce_done = false;
public:
    S_queue(int Capacity) : capacity(Capacity){}

    void q_push(int item){
        unique_lock<mutex> ul(mtx);
        not_full.wait(ul, [this]{return q.size() < capacity;});
        q.push(item);
        cout << "生产者线程" << this_thread::get_id() << "生产了一个数据：" << item << endl;
        not_empty.notify_one();
    }

    bool q_pop(){
        unique_lock<mutex> ul(mtx);
        not_empty.wait(ul, [this]{return produce_done || !q.empty();});
        if(produce_done && q.empty()){
            return false;
        }
        int val = q.front();
        q.pop();
        cout << "消费者线程" << this_thread::get_id() << "消费了一个数据：" << val << endl;
        not_full.notify_one();
        return true;
    }

    void ProduceDone(){
        unique_lock<mutex> ul(mtx);
        produce_done = true;
        not_empty.notify_all();
    }
};

void produce(S_queue& s_q, const int& start_id){
    for(int i = 1; i <= 10; i++){
        s_q.q_push(10*start_id + i);
    }
}

void consume(S_queue& s_q){
    while(true){
        bool consume_done = !s_q.q_pop();
        if(consume_done) break;
    }
}

int main(){
    auto start = chrono::steady_clock::now();
    S_queue s_q(10);
    vector<thread> producer, consumer;
    for(int i = 0; i < 2; i++){
        producer.emplace_back(produce, ref(s_q), i);
    }

    for(int i = 0; i < 3; i++){
        consumer.emplace_back(consume, ref(s_q));
    }

    for(int i = 0; i < 2; i++){
        producer[i].join();
    }
    s_q.ProduceDone();
    for(int i = 0; i < 3; i++){
        consumer[i].join();
    }
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "程序执行结束，耗费时间：" << duration.count() << "微秒！" << endl;
    return 0;
}