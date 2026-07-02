#include <iostream>
#include <string>
using namespace std;

class Singer{
public:
    string Sname;
    int Sage;
    Singer(string sname, int sage):Sname(sname), Sage(sage){
        cout << "Singer的构造函数被调用" << endl;
    }
    ~Singer(){
        cout << "Singer的析构函数被调用" << endl;
    }
};

class Team{
public:
    Singer S1;
    string Tname;
    Team(string sname, int sage, string tname):S1(sname,sage),Tname(tname){
        cout << "Team的构造函数被调用" << endl;
    }
    ~Team(){
        cout << "Team的析构函数被调用" << endl;
    }
};

void test01(){
    Team T1("YOCO", 22, "DCT");
}

int main(){
    test01();
}