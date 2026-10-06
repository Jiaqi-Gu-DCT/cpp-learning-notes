//类成员函数作为线程入口：pthread必须进行静态中转而std::thread则方便许多
#include<iostream>
#include<thread>
using namespace std;

class Worker{
private:
    string name;
    int count;
public:
    Worker(string n, int c): name(n),count(c){}

    void doWork(){
        cout << "工人：" << name << "， 现在进行共计：" << count << "单位的工作！" << endl;
        for(int i = 1; i <= count; i++){
            cout << "当前正在进行第 " << i << " 单位的工作。" << endl;
        }
        cout << "所有工作均已完成！" << endl;
    }
};

int main(){
    Worker w1 ("经合", 10);
    thread t1 (&Worker::doWork, &w1);
    t1.join();
    return 0;
}
    
