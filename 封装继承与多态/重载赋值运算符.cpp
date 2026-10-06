#include <iostream>
using namespace std;

class Singer{
friend void test01();
private:
    int* S_Age;
public:
    Singer(int age){
        S_Age = new int (age);
    }

    ~Singer(){
        if(S_Age != nullptr){
            delete S_Age;
            S_Age = nullptr;
        }
    }
    // 深拷贝
    Singer& operator = (Singer& s1){
        if(S_Age != nullptr){
            delete S_Age;
            S_Age = nullptr;
        }
        S_Age = new int (*s1.S_Age);
        return *this;
    }
};

void test01(){
    Singer Yoco(22);
    Singer Chiang(23);
    Singer Long(23);
    Singer Kai(22);
    Yoco = Chiang = Long = Kai;
    cout << "DCT大家的年龄为：" << *Yoco.S_Age << endl;

}

int main(){
    test01();
}