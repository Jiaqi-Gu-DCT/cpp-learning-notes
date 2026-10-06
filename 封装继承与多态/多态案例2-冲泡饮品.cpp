#include <iostream>
using namespace std;

// 基类(抽象类)：制作饮品
class MakeDrinks{
public:
    void boil(){
        cout << "煮水" << endl;
    }
    virtual void brew() = 0;
    void pour(){
        cout << "倒入杯中" << endl;
    }
    virtual void add() = 0;
};
// 子类1 ： 制作咖啡
class MakeCoffee: public MakeDrinks{
public:
    void brew() override {
        cout << "加入咖啡粉" << endl;
    }
    void add() override {
        cout << "加入抹茶粉和牛奶" << endl;
    }
};
// 子类2 ： 制作茶
class MakeTea: public MakeDrinks{
public:
    void brew() override {
        cout << "加入茉莉花茶包" << endl;
    }
    void add() override {
        cout << "加入方糖" << endl;
    }
};
// 测试制作咖啡
void Coffee(){
    MakeDrinks * MD = new MakeCoffee();
    MD->boil();
    MD->brew();
    MD->pour();
    MD->add();
    delete MD;
    MD = nullptr;
}
// 测试制作茶
void Tea(){
    MakeDrinks * MD = new MakeTea();
    MD->boil();
    MD->brew();
    MD->pour();
    MD->add();
    delete MD;
    MD = nullptr;
}

int main(){
    cout << "-----第一位客人要喝咖啡，制作中-----" << endl;
    Coffee();
    cout << "-----客人很满意-----" << endl;
    cout << "-----第二位客人要喝茶，制作中-----" << endl;
    Tea();
    cout << "-----结束-----" << endl;
}