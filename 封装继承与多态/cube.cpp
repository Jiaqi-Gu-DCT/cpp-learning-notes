#include <iostream>
using namespace std;

class Cube{
private:
    int m_H, m_L, m_W;
public:
    void setH(int H){
        m_H = H;
    }
    int getH(){
        return m_H;
    }

    void setL(int L){
        m_L = L;
    }
    int getL(){
        return m_L;
    }

    void setW(int W){
        m_W = W;
    }
    int getW(){
        return m_W;
    }

    int CS(){
        return (m_L*m_H+m_L*m_W+m_H*m_W)*2;
    }
    int CV(){
        return m_H*m_L*m_W;
    }
};

bool check(Cube C1, Cube C2){
    return (C1.getH()==C2.getH())&&(C1.getL()==C2.getL())&&(C1.getW()==C2.getW());
}

int main(){
    Cube C1, C2;
    C1.setH(2);
    C1.setL(3);
    C1.setW(4);
    cout << "第一个立方体的表面积：" << C1.CS() << "平方米" << endl;

    C2.setH(5);
    C2.setL(6);
    C2.setW(7);
    cout << "第二个立方体的体积：" << C2.CV() << "立方米" << endl;

    if(check(C1,C2)){
        cout << "二者相同" << endl;
    }else{
        cout << "二者不同" << endl;
    }

}