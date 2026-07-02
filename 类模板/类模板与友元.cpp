#include <iostream>
using namespace std;

template<class T1, class T2>
class Person;

template<class T1, class T2>
void printPerson2(Person<T1, T2> p){
    cout << "类外实现" << endl;
    cout << "名前：" << p.m_Name << "  年龄：" << p.m_Age << endl;
}

template<class T1, class T2>
class Person{
private:
    T1 m_Name;
    T2 m_Age;
public:
    Person(T1 n, T2 a):m_Name(n),m_Age(a){}

    // 全局函数类内实现
    friend void printPerson1(Person<T1, T2> p){
        cout << "类内实现" << endl;
        cout << "名前：" << p.m_Name << "  年龄：" << p.m_Age << endl;
    }

    // 全局函数类外实现
    friend void printPerson2<>(Person<T1, T2> p);
};

void test(){
    Person<string, int> p1("YOCO", 22);
    printPerson1(p1);
    printPerson2(p1);
}

int main(){
    test();
}