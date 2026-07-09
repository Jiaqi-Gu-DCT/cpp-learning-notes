// 题目要求：启动 2 个生产者线程，每个线程生产 10 个整数（共20个数据）；启动 3 个消费者线程，持续从队列取数据；
// 所有数据生产完毕后，消费者线程能正常检测到结束信号并退出，不能出现永久阻塞卡死；
// 全程保证线程安全，输出不要求顺序，但不能出现重复消费、漏消费。
#include <iostream>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <queue>
#include <thread>
using namespace std;

class S_queue{
friend void consume(S_queue& s_q);
private:
    queue<int> q;
    mutex mtx;
    int capacity;
    condition_variable not_full, not_empty;
    bool done = false;
public:
    S_queue(int c) : capacity(c){}

    void q_push(int item){
        unique_lock<mutex> ul(mtx);
        not_full.wait(ul, [this]{return q.size() < capacity;});
        q.push(item);
        cout << "生产者" << this_thread::get_id() << "传入了一个数据，值为：" << item << endl;
        not_empty.notify_one();
    }

    bool q_pop(){
        unique_lock<mutex> ul(mtx);
        not_empty.wait(ul, [this]{return !q.empty() || done;});
        if(q.empty() && done){
            return false;
        }
        int val = q.front();
        q.pop();
        cout << "消费者" << this_thread::get_id() << "拿走了一个数据，值为" << val << endl;
        not_full.notify_one();
        return true;
    }

    void produce_done(){
        unique_lock<mutex> ul(mtx);
        done = true;
        not_empty.notify_all();
    }
};

void produce(S_queue& s_q, int start_id){
    for(int i = 1; i <= 10; i++){
        s_q.q_push(10*start_id + i);
    }
}

void consume(S_queue& s_q){
    while(true){
        bool ref = s_q.q_pop();
        if(!ref) break;
    }
}

int main(){
    auto start = chrono::steady_clock::now();
    S_queue SQ(10);
    vector<thread> producers, consumers;

    for(int i = 0; i < 2; i++){
        producers.emplace_back(produce, ref(SQ), i);
    }
    for(int i = 0; i < 3; i++){
        consumers.emplace_back(consume, ref(SQ));
    }

    for(auto& p : producers){
        p.join();
    }
    SQ.produce_done();
    for(auto& c : consumers){
        c.join();
    }

    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "程序执行结束，耗时：" << duration.count() << "微秒。" << endl;
    return 0;
}