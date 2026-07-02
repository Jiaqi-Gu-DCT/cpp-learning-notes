#include<iostream>
using namespace std;
// 抽象基类
class CPU{
public:
    virtual void calculate() = 0;
    virtual ~CPU(){};
};

class VideoCard{
public:
    virtual void display() = 0;
    virtual ~VideoCard(){};
};

class Memory{
public:
    virtual void storage() = 0;
    virtual ~Memory(){};
};
// 零件厂商：英特尔
class IntelCPU: public CPU{
public:
    void calculate()override{
        cout << "来自Intel的CPU正在执行计算任务～" << endl;
    }
};

class IntelVideoCard: public VideoCard{
public:
    void display()override{
        cout << "来自Intel的显卡正在执行渲染任务～" << endl;
    }
};

class IntelMemory: public Memory{
public:
    void storage()override{
        cout << "来自Intel的内存条正在执行存储任务～" << endl;
    }
};
// 零件厂商：辉达
class NvidiaCPU: public CPU{
public:
    void calculate()override{
        cout << "来自NVIDIA的CPU正在执行计算任务～" << endl;
    }
};

class NvidiaVideoCard: public VideoCard{
public:
    void display()override{
        cout << "来自NVIDIA的显卡正在执行渲染任务～" << endl;
    }
};

class NvidiaMemory: public Memory{
public:
    void storage()override{
        cout << "来自NVIDIA的内存条正在执行存储任务～" << endl;
    }
};
// 电脑类
class computer{
private:
    CPU* cpu;
    VideoCard* vcd;
    Memory* meo;
public:
    computer(CPU* cpu, VideoCard* vcd, Memory* meo){
        this->cpu = cpu;
        this->vcd = vcd;
        this->meo = meo;
        cout << "本台电脑组装完成" << endl;
    }
    void work(){
        cout << "本台电脑开始工作" << endl;
        cpu->calculate();
        vcd->display();
        meo->storage();
        cout << "全部任务执行结束" << endl;
    }
    ~computer() {
        if (cpu != nullptr) {
            delete cpu;
            cpu = nullptr;
            cout << "CPU已释放" << endl;
        }
        if (vcd != nullptr) {
            delete vcd;
            vcd = nullptr;
            cout << "显卡已释放" << endl;
        }
        if (meo != nullptr) {
            delete meo;
            meo = nullptr;
            cout << "内存已释放" << endl;
        }
    }
};
// 测试组装电脑
void Inter(){
    cout << "来自英特尔的电脑正在组装" << endl;
    CPU* cpu = new IntelCPU();
    VideoCard* vcd = new IntelVideoCard();
    Memory* meo = new IntelMemory();
    computer intel(cpu, vcd, meo);
    cout << "来自英特尔的电脑组装成功" << endl;
    intel.work();
}

void NVIDIA(){
    cout << "来自辉达的电脑正在组装" << endl;
    CPU* cpu = new NvidiaCPU();
    VideoCard* vcd = new NvidiaVideoCard();
    Memory* meo = new NvidiaMemory();
    computer nvidia(cpu, vcd, meo);
    cout << "来自辉达的电脑组装成功" << endl;
    nvidia.work();
}

int main(){
    Inter();
    NVIDIA();
}