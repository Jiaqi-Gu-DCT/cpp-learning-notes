#include <iostream>
using namespace std;

template<class T1, class T2>
class Person{
public:
    Person(T1 a, T2 n);
    // 类内的局部函数
    void showInfo();
    // 全局函数
    // void printPerson(Person<T1,T2> p){...}
    T1 m_Age;
    T2 m_Name;
};

template<class T1, class T2>
Person<T1, T2>::Person(T1 a, T2 n){
    this->m_Age = a;
    this->m_Name = n;
}

template<class T1, class T2>
void Person<T1, T2>::showInfo(){
    cout << "名前：" << this->m_Name << "  年龄：" << this->m_Age << endl;
}