#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;

class Person{
public:
    int mAge;
    string mName;

    Person(int age, string name){
        mAge = age;
        mName = name;
    }
};

void MyPrint(Person& p){
    cout << "名前：" << p.mName << " 年龄：" << p.mAge << endl;
}

vector<Person> v;

void test01(){
    Person p1(22, "Yoco");
    Person p2(23, "Chiang");
    Person p3(22, "Long");
    Person p4(23, "Kai");
    v.push_back(p1);   
    v.push_back(p2);  
    v.push_back(p3);  
    v.push_back(p4); 

    cout << "第一种遍历方式" << endl;
    
    // 第一种遍历方式
    for(vector<Person>::iterator it = v.begin(); it != v.end(); it++){
        cout << "名前：" << it->mName << " 年龄：" << it->mAge << endl;
    }
}

void test02(){
    cout << "第二种遍历方式" << endl;
    
    // 第二种遍历方式
    for_each(v.begin(), v.end(), MyPrint);
}

int main(){
    test01();
    test02();
}