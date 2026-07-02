#include <iostream>
#include <thread>
#include <vector>
using namespace std;
const int THREAD_NUM = 20;
const int PER_THREAD_NUM = 5;

int partialCalc(int index){
    int num = 0;
    int start = index*PER_THREAD_NUM+1;
    int end = (index + 1) * PER_THREAD_NUM;
    for(int i = start; i <= end; i++){
        num += i;
    }
    return num;
}

int main(){
    vector<thread> threads;
    long long total = 0;
    vector<int> seg(THREAD_NUM);

    for(int i = 0; i < THREAD_NUM; i++){
        threads.emplace_back([i,&seg](){
            seg[i] = partialCalc(i);
        });
    }

    for(auto& t : threads){
        t.join();
    }

    for(int val : seg){
        total += val;
    }
    
    cout << "从1加到" << THREAD_NUM * PER_THREAD_NUM << "的值为： " << total << endl;
}