//体会detach的生命周期陷阱
//深刻理解「分离线程的生命周期必须小于等于访问变量的生命周期」
//detach线程绝对不能捕获 / 访问局部变量的指针、引用；如果必须传数据，要么用值拷贝，要么用堆内存 / 全局变量。
#include<iostream>
#include<thread>
#include<chrono>
using namespace std;

void startWork(){
    string localMsg = "这是局部变量的消息";
    thread t(
        [&localMsg](){
            this_thread::sleep_for(chrono::seconds(2));
            cout << "线程使用：" << localMsg << endl;
        }
    );
    //注意方式
    t.join();
}

int main(){
    startWork();
    this_thread::sleep_for(chrono::seconds(3));
    cout << "主线程结束！" << endl;
    return 0;
}