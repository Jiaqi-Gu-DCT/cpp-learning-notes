#include <iostream>
using namespace std;

class person{
friend person operator+ (const person& p1, const person& p2);
private:
    int age;
public:
    person(int a): age(a){}
    void setAge(int age){
        this->age = age;
    }

    int getAge(){
        return this->age;
    }
};

person operator+ (const person& p1, const person& p2){
    person temp(0);
    temp.age = p1.age + p2.age;
    return temp;
}

void test01(){
    person Yoco(22);
    person Long(23);
    person ZHOU(0);
    ZHOU = Yoco + Long;
    cout << "ZHOU的年龄是" << ZHOU.getAge() << endl;
    ZHOU = ZHOU + Yoco +Long;
    cout << "ZHOU的年龄是" << ZHOU.getAge() << endl;
}

int main(){
    test01();
}