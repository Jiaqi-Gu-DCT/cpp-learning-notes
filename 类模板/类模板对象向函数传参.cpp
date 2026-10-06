// 在这里及以后我们规定只能以指定传入类型方式
#include <iostream>
using namespace std;

template<class NameType, class AgeType>
class student{
public:
    NameType name;
    AgeType age;
    student(NameType n, AgeType a): name(n),age(a){}
    void showInfo() const{
        cout << "名前：" << name << ", 年龄：" << age << endl;
    }
};

// 指定传入类型，最常用
void Myprint(student<string, int>& s1){
    cout << "这是指定类型传参" << endl;
    s1.showInfo();
}

// 函数模板化
template <class T, class Y>
void Print(student<T,Y>& s1){
    cout << "这是利用了函数模板来传参" << endl;
    s1.showInfo();
}

void test(){
    student<string, int> s1("YOCO", 22);
    student<string,double> s2("KAI", 23);
    Myprint(s1);
    // Myprint(s2);会报错
    Print(s1);
    Print(s2);
}

int main(){
    test();
}