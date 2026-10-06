//多参数传递
//按照景和宫要求 学生需掌握原生pthread和C++中的std::thread两种写法 更深入理解多线程
#include <iostream>
#include <thread>
using namespace std;

struct Student{
    int Id;
    string Name;
    int Score;
};

void Student_diaplay (Student* s){
    cout << "学号：" << s->Id << ",姓名：" << s->Name << ",成绩：" << s->Score << endl;
}

int main(){
    Student s1 {01, "YOCO", 100};
    thread t1(Student_diaplay, &s1);
    t1.join();
    return 0;
}