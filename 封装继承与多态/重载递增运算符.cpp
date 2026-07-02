#include <iostream>
using namespace std;

class MyInteger{
friend void test01();
friend void test02();
private:
    int myint;
public:
    MyInteger(int a){
        myint = a;
    }

    // 前置
    MyInteger& operator ++(){
        this->myint++;
        return *this;
    }
    // 后置
    MyInteger operator ++(int){
        MyInteger temp = *this;
        this->myint++;
        return temp;
    }
};

void test01(){ // 前置的测试
    MyInteger m1(100);
    cout << "m1的值为：" << m1.myint << endl;
    ++m1;
    cout << "修改后m1的值为：" << m1.myint << endl;

}

void test02(){ // 后置的测试
    MyInteger m1(100);
    cout << "m1的值为：" << m1.myint << endl;
    m1++;
    cout << "修改后m1的值为：" << m1.myint << endl;

}

int main(){
    test02();
}