#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<deque>
#include<algorithm>
#include<ctime>
using namespace std;
//创建选手类
class contestant{
friend void setGrade(vector<contestant>& v);
friend void showGrade(const vector<contestant>& v);
private:
    string name;
    int grade;
public:
    contestant(string name, int grade){
        this->grade = grade;
        this->name = name;
    }
};
// 创建5位选手
void createContestant(vector<contestant>& v){
    string nameseed = "ABCDE";
    for(int i = 0; i < 5; i++){
        string name = "选手";
        name += nameseed[i];
        int grade = 0;
        v.push_back(contestant(name,grade));
    }

}
//评分
void setGrade(vector<contestant>& v){
    for(vector<contestant>::iterator it = v.begin(); it != v.end(); it++){
        deque<int>d;
        for(int i = 0; i < 10; i++){
            int score = rand()% 41 + 60;
            d.push_back(score);
        }
        sort(d.begin(),d.end());
        d.pop_back();
        d.pop_front();

        int sum = 0;
        for(deque<int>::iterator dit = d.begin(); dit != d.end(); dit++){
            sum += *dit;
        }
        int avg = sum/d.size();
        it->grade = avg;
    }

}
//展示分数
void showGrade(const vector<contestant>& v){
    for(vector<contestant>::const_iterator it = v.begin(); it != v.end(); it++){
        cout << "姓名：" << it->name << " 成绩：" << it->grade << endl;
    }
}


int main(){
    srand((unsigned int)time(NULL));
// 1.创建5位选手
    vector<contestant> v;
    createContestant(v);
// 2.10名裁判为这5位选手打分
    setGrade(v);
    showGrade(v);
}