// 在这个部分，我们主要学习当父类是一个模板时，子类的声明问题；
// 以及想要灵活指定父类模板参数类型，子类的处理方式。
#include <iostream>
using namespace std;

template<class NameType, class AgeType>
class Person{
public:
    NameType name;
    AgeType age;
    Person(NameType n, AgeType a): name(n), age(a){}
    virtual void showInfo() = 0;
};

class Student: public Person<string, int>{
public:
    // 重点1
    Student(string n, int a):Person<string, int>(n,a){}
    void showInfo()override{
        cout << "学生  名前：" << name << "，年龄：" << age << endl;
    }
};

template<class T1, class T2>
class Singer: public Person<T1, T2>{
public:
    // 重点2
    Singer(T1 n, T2 a): Person<T1, T2>(n,a){}
    void showInfo() override{
        // 重点3
        cout << "歌手  名前：" << this->name << "，年龄：" << this->age << endl;
    }
};

void test(){
    Student s1("YOCO",22);
    Singer<string,double> s2("KAI", 23);

    s1.showInfo();
    s2.showInfo();
}

int main(){
    test();
}