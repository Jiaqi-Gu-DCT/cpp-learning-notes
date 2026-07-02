#include <iostream>
using namespace std;

// 计算器基类(默认num1是第一操作数，num2是第二操作数)
class abstractCaculator{
public:
    virtual int getResult(){
        return 0;
    }
    int num1, num2;
};
// 加法的实现
class addCaculator: public abstractCaculator{
public:
    int getResult() override {
        return num1 + num2;
    }
};
// 减法的实现
class subCaculator: public abstractCaculator{
public:
    int getResult() override {
        return num1 - num2;
    }
};
// 乘法的实现
class mulCaculator: public abstractCaculator{
public:
    int getResult() override {
        return num1 * num2;
    }
};
// 测试函数，测试计算器各功能的实现效果：
void test01(){
    abstractCaculator * ab = new addCaculator();
    ab->num1 = 22;
    ab->num2 = 18;
    cout << "num1 + num2 = " << ab->getResult() << endl;
    delete ab;

    ab = new subCaculator();
    ab->num1 = 22;
    ab->num2 = 18;
    cout << "num1 - num2 = " << ab->getResult() << endl;
    delete ab;

    ab = new mulCaculator();
    ab->num1 = 22;
    ab->num2 = 18;
    cout << "num1 * num2 = " << ab->getResult() << endl;
    delete ab;
    ab = nullptr;
}

int main(){
    test01();
}