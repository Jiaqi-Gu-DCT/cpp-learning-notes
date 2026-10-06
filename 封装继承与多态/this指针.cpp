#include <iostream>
#include <string>
using namespace std;

class Student{
public:
    Student(int no, int age, string name){
        this->no = no;
        this->age = age;
        this->name = name;
    }
    ~Student(){}
    Student& addAge(Student &s){
        age += s.age;
        return *this;
    }

    int no, age;
    string name;
};

void test01(){
    Student s1(01, 13, "朱祁钰");
    cout << s1.name <<"的年龄是：" << s1.age << endl;
}

void test02(){
    Student s2(02, 14, "朱祁镇");
    cout << s2.name <<"的年龄是：" << s2.age << endl;
    Student s3(03, 2, "朱见深");
    s2.addAge(s3);
    cout << s2.name <<"的年龄是：" << s2.age << endl;
    s2.addAge(s3).addAge(s3).addAge(s3).addAge(s3);
    cout << s2.name <<"的年龄是：" << s2.age << endl;
}

int main(){
    test01();
    cout << "--------------下一个测试：-----------------" << endl;
    test02();
}