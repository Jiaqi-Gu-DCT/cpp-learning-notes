#include <iostream>
using namespace std;

class MyInteger{
friend void test01();
friend void test02();
public:
    MyInteger(){
        myint = 100;
    }
    // 前置
    MyInteger& operator --(){
        myint--;
        return *this;
    }
    // 后置
    MyInteger operator --(int){
        MyInteger temp = *this;
        myint--;
        return temp;
    }
private:
    int myint;

};

void test01(){
    MyInteger m1;
    cout << "m1的myint值为：" << m1.myint << endl;
    m1--;
    cout << "m1的myint值为：" << m1.myint << endl;
}

void test02(){
    MyInteger m1;
    cout << "m1的myint值为：" << m1.myint << endl;
    --m1;
    cout << "m1的myint值为：" << m1.myint << endl;
}

int main(){
    test02();
}