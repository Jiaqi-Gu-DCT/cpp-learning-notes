// 题目要求：按照年龄升序，若年龄相同则按照身高降序
// 1.Yoco,22,181
// 2.Chiang,23,185
// 3.Long,23,176
// 4.Kai,23,170
#include<iostream>
#include<string>
#include<list>
using namespace std;
//学生类
class Student{
public:
    string name;
    int age;
    int height;
public:
    Student(string n, int a, int h):name(n),age(a),height(h){}
};
// 排序规则
bool myCompare(const Student& s1,const Student& s2){
    return( (s1.age < s2.age) || ( (s1.age == s2.age) && (s1.height > s2.height) ) );
}
// 输出学生
void printStudent(const list<Student>& s){
    for(list<Student>::const_iterator sit = s.begin(); sit != s.end(); sit++){
        cout << "姓名：" << sit->name << " 年龄：" << sit->age << " 身高：" << sit->height << endl;
    }
}
// 创建学生
void createStudent(list<Student>& s){
    Student s1("Yoco", 22, 181);
    s.push_back(s1);
    Student s2("Chiang", 23, 185);
    s.push_back(s2);
    Student s3("Kai", 23, 170);
    s.push_back(s3);
    Student s4("Long", 23, 176);
    s.push_back(s4);
}
// 排序
void sortStudent(list<Student>& s){
    cout << "排序前：" << endl;
    printStudent(s);
    s.sort(myCompare);
    cout << "排序后：" << endl;
    printStudent(s);
}

int main(){
    list<Student> stu;
    createStudent(stu);
    sortStudent(stu);
}