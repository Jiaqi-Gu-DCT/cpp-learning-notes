#include <iostream>
using namespace std;

void myPrint(int a){
    cout << "调用了普通函数" << endl;
}

template<typename T>
void myPrint(T a){
    cout << "调用了函数模板" << endl;
}

template<typename T>
void myPrint(T a, T b){
    cout << "调用了重载的函数模板" << endl;
}

int main(){
    //myPrint();
    //myPrint<>(2);
    //myPrint<>(1,2);
    //myPrint('a', 'b');
    myPrint("string");

}