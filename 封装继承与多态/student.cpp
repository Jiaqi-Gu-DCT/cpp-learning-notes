#include <iostream>
using namespace std;

class Student{
    public:
    int no; //学号
    string name; //姓名
    
    void setNo(int a){
        no = a;
    }

    void setName(string b){
        name = b;
    }

    void showInfo(){
        cout << "学号：" << no << " 姓名：" << name << endl;
    }
};

int main(){
    Student s1;
    s1.setNo(22);
    s1.setName("YOCO");
    s1.showInfo();
}