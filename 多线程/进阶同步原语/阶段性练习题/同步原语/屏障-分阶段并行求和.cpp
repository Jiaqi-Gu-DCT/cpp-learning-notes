// 题目要求：有一个大小为100的整数数组，元素为1~100，启动4个线程分两阶段计算求和：
//     阶段1：每个线程分别计算自己负责的1/4数组的局部和；
//     阶段2：所有线程完成阶段1后，统一累加所有局部和，输出最终总和。
#include <iostream>
#include <barrier>
#include <thread>
#include <vector>
#include <mutex>
using namespace std;

mutex mtx;
const int THREAD_NUM = 4;
const int ARR_SIZE = 100;
const int PARTIAL_SIZE = 25;

barrier bar(4);
vector<int> num(ARR_SIZE);
vector<int> partial_results(THREAD_NUM);

void worker(int id){
    int start = id * PARTIAL_SIZE;
    int end = (id + 1) * PARTIAL_SIZE - 1;
    int local_sum = 0;
    for(int i = start; i <= end; i++){
        local_sum += num[i];
    }
    partial_results[id] = local_sum;
    unique_lock<mutex> ul(mtx);
    cout << "【线程" << id <<"】的结果为：" << local_sum << endl;
    ul.unlock();
    bar.arrive_and_wait();

    if(id == 0){
        int total = 0;
        for(int i = 0; i < THREAD_NUM; i++){
            total += partial_results[i];
        }
        cout << "分阶段并行求和的结果为：" << total << endl;
    }
}

int main(){
    vector<thread> threads;
    for(int i = 1; i <= 100; i++){
        num[i-1] = i;
    }

    for(int i = 0; i < 4; i++){
        threads.emplace_back(worker, i);
    }

    for(auto& t : threads){
        t.join();
    }
    return 0;
}
