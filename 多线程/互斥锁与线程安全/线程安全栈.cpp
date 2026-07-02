// 本题要求：采用lock_guard实现一个简单的线程安全的栈(支持push、pop和获取栈顶元素)
// pop操作：栈空时返回false，非空则弹出元素并通过引用带出结果
// 验证：开启两个线程push和pop，程序不崩溃且数据不丢失。 回顾函数模板。
#include <iostream>
#include <thread>
#include <mutex>
#include <stack>
using namespace std;

template<typename T>
class ThreadSafeStack{
private:
    stack<T> stk;
    mutable mutex mtx; // 在常函数中也可以正常修改
public:
    void push(T val){
        lock_guard<mutex> lg(mtx);
        stk.push(val);
    }
    bool pop(T& val){
        lock_guard<mutex> lg(mtx);
        if(stk.empty()){
            return false;
        }
        val = stk.top();
        stk.pop();
        return true;
    }

    // 常函数，无法修改成员属性，但是加了关键字mutable的可以修改
    bool empty() const{
        lock_guard<mutex> lg(mtx);
        return stk.empty();
    }
};

int main(){
    ThreadSafeStack<int> stk1;
    const int NUM = 10000;

    thread t1([&](){
        for(int i = 0; i < NUM; i++) stk1.push(i);
    });
    thread t2([&](){
        for(int i = 0; i < NUM; i++) stk1.push(i + NUM);
    });

    int sum1 = 0, sum2 = 0;
    thread t3([&](){
        int val;
        while(stk1.pop(val)) sum1 += val;
    });
    thread t4([&](){
        int val;
        while(stk1.pop(val)) sum2 += val;
    });

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    cout << "弹出的元素总和为：" << sum1 + sum2 << endl;
    return 0;
}
// 在学习完生产者-消费者问题实现后，回头复习这份代码 看看有什么新的理解？