// 这一部分我们练习在类外实现类模板的成员函数，如构造函数和普通函数
#include <iostream>
using namespace std;

template<class T1, class T2>
class Person{
public:
    T1 name;
    T2 age;
    Person(T1 n, T2 a);
    void showInfo();
};

template<class T1, class T2>
Person<T1, T2>::Person(T1 n, T2 a){
    this->name = n;
    this->age = a;
}

template<class T1,class T2>
void Person<T1, T2>::showInfo(){
    cout << "姓名：" << this->name << "年龄：" << this->age << endl;
}

void test(){
    Person<string, int> p1("Yoco", 22);
    p1.showInfo();
}

int main(){
    test();
}