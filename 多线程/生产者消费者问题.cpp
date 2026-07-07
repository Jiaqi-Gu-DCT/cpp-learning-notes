#include <iostream>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <chrono>
using namespace std;

class BoundedBuffer{
private:
    mutex mtx;
    queue<int> buffer;
    condition_variable not_full, not_empty;
    int capacity;
public:
    BoundedBuffer(int cap): capacity(cap){}

    // 生产者生产过程
    void produce(int item){
        unique_lock<mutex> ul(mtx);
        not_full.wait(ul, [this]{return buffer.size() < capacity;});
        buffer.push(item);
        not_empty.notify_one();
    }

    // 消费者消费过程
    int consume(){
        unique_lock<mutex> ul(mtx);
        not_empty.wait(ul, [this]{return !buffer.empty();});
        int item = buffer.front();
        buffer.pop();
        not_full.notify_one();
        return item;
    }
};
// 生产者函数
void producer(BoundedBuffer& buf){
    for(int i = 0; i < 10; i++){
        buf.produce(i);
        cout << "Produced:" << i << endl;
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

// 消费者函数
void consumer(BoundedBuffer& buf){
    for(int i = 0; i < 10; i++){
        buf.consume();
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

int main(){
    BoundedBuffer buffer(3);
    thread prod(producer, ref(buffer));
    thread cons(consumer, ref(buffer));

    prod.join();
    cons.join();

    cout << "all the task of producing and consuming are finished." << endl;
    return 0;
}