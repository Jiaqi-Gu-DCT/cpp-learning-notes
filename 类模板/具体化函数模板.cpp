// 加和函数
#include <iostream>
using namespace std;

class Singer{
public:
    int age;
    string name;
    Singer(int a, string b):age(a), name(b){}
    void showInfo(){
        cout << "名前：" << this->name << "  年龄：" << this->age << endl;
    }
};

template<class T>
int Add(T& a, T& b){
    return a+b;
}

template<>
int Add(Singer& s1, Singer& s2){
    return s1.age+s2.age;
}

void test(){
    Singer s1(22, "Yoco");
    Singer s2(23, "KAI");
    int sum = Add(s1, s2);
    cout << "二位歌手年龄之和为：" << sum << endl;
}

int main(){
    test();
}