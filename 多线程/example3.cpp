// 线程返回值获取（以累加为例）
#include<iostream>
#include<thread>
using namespace std;

void CalcSum (int arg, int& sum){
    sum = 0;
    for(int i = 1; i <= arg; i++){
        sum += i;
    }
}

int main(){
    int result;
    thread t1(CalcSum, 100, ref(result));
    t1.join();

    cout << "结果是：" << result << endl;
    return 0;
}