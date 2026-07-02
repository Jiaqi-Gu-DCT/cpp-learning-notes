#include <iostream>
#include <cmath>
using namespace std;

class Circle{
private:
    double C_x, C_y, r;
public:
    void setC_x(double x){
        C_x = x;
    }
    double getC_x(){
        return C_x;
    }
    void setC_y(double y){
        C_y = y;
    }
    double getC_y(){
        return C_y;
    }
    void setR(double R){
        r = R;
    }
    double getR(){
        return r;
    }
};

class Point{
private:
    double P_x, P_y;
public:
    void setP_x(double x){
        P_x = x;
    }
    double getP_x(){
        return P_x;
    }
    void setP_y(double y){
        P_y = y;
    }
    double getP_y(){
        return P_y;
    }
    
    double distance(Circle C){
        return pow(pow((P_x - C.getC_x()), 2)+pow((P_y-C.getC_y()), 2),0.5);
    }
};

int main(){
    Circle C1;
    Point P1;
    C1.setC_x(2.455);
    C1.setC_y(1.234);
    C1.setR(2.314);
    P1.setP_x(3.525);
    P1.setP_y(3.141);
    double dis = P1.distance(C1);
    cout << "点到圆的距离为：" << dis << endl;
    if(dis == C1.getR()){
        cout << "该点在圆上" << endl;
    }else if(dis < C1.getR()){
        cout << "该点在圆内" << endl;
    }else{
        cout << "该点在圆外" << endl;
    }
}