// 题目要求：实现一个容量为5的线程安全有界整数队列：
// 1.生产者和消费者线程数都为1，生产者向队列中放入1~10的整数
// 2.应采取双条件变量
#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>
using namespace std;

class S_queue{
private:
    queue<int> q;
    mutex mtx;
    condition_variable not_empty, not_full;
    int capacity;

public:
    S_queue(int cap) : capacity(cap){}

    void q_push(int val){
        unique_lock<mutex> ul(mtx);
        not_full.wait(ul, [this]{return this->q.size() < capacity;});
        q.push(val);
        cout << "生产者：" << this_thread::get_id() << "生产了一个数据，值为：" << val << endl;
        not_empty.notify_one();
    }

    void q_pop(){
        unique_lock<mutex> ul(mtx);
        not_empty.wait(ul, [this]{return !this->q.empty();});
        int val = q.front();
        q.pop();
        cout << "消费者：" << this_thread::get_id() << "消费了一个数据，值为：" << val << endl;
        not_full.notify_one();
    }

};

void produce(S_queue& s_q){
    for(int i = 1; i <= 10; i++){
        s_q.q_push(i);
    }
}

void consume(S_queue& s_q){
    for(int i = 0; i < 10; i++){
        s_q.q_pop();
    }
}

int main(){
    auto start = chrono::steady_clock::now();
    S_queue s_q(5);
    thread prod(produce, ref(s_q));
    thread cons(consume, ref(s_q));
    prod.join();
    cons.join();
    cout << "全部线程执行完毕" << endl;
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "程序执行耗费时间为：" << duration.count() << "微秒。" << endl;
    return 0;
}