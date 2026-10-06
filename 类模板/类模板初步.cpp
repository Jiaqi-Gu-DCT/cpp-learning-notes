// 这部分我们一起探讨类模板基本语法和类模板与函数模板之间的区别；还有类模板中成员函数的创建时机
#include <iostream>
using namespace std;

template<class NameType>
class Student{
public:
    int age;
    NameType name;
    Student(int a, NameType b):age(a),name(b){}
};
void test(){
    Student<string> s1(12, "chen");
    cout << "名前：" << s1.name << endl << "年龄：" << s1.age << endl;
}
// 类模板中成员函数只在调用时才创建