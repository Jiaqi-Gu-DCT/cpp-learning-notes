// Problem Requirements: 
// Start three threads that only print A, B, and C respectively. 
// They must print in a strict cyclic sequence of ABCABCABC... 
// Each letter should be printed for 5 rounds. Condition variables must be used to implement the logic,
// and sleep cannot be utilized to control the printing order.
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
using namespace std;

class Document{
private:
    mutex mtx;
    condition_variable input_A, input_B, input_C;
    int capacity;
    int flag = 0; // 0--A; 1--B; 2--C
public:
    Document(int Capacity) : capacity(Capacity){};

    void InputA(){
        unique_lock<mutex> ul(mtx);
        input_A.wait(ul, [this]{return flag == 0;});
        cout << 'A';
        flag = 1;
        input_B.notify_one();
    }

    void InputB(){
        unique_lock<mutex> ul(mtx);
        input_B.wait(ul, [this]{return flag == 1;});
        cout << 'B';
        flag = 2;
        input_C.notify_one();
    }

    void InputC(){
        unique_lock<mutex> ul(mtx);
        input_C.wait(ul, [this]{return flag == 2;});
        cout << 'C';
        flag = 0;
        input_A.notify_one();
    }
};

void pushA(Document& d){
    for(int i = 0; i < 5; i++){
        d.InputA();
    }
}

void pushB(Document& d){
    for(int i = 0; i < 5; i++){
        d.InputB();
    }
}

void pushC(Document& d){
    for(int i = 0; i < 5; i++){
        d.InputC();
    }
}

    
int main(){
    Document now(15);
    thread tA(pushA, ref(now));
    thread tB(pushB, ref(now));
    thread tC(pushC, ref(now));
    tA.join(), tB.join(), tC.join();
    cout << endl << "FINISHED！"  << endl;
    return 0;
}

// In most interviews, written tests and daily development work, 
// a single condition variable paired with notify_all is the default first choice. 
// There is no need to adopt the chained writing style with multiple condition variables 
// unless precise wake-up and ultimate performance are explicitly required.