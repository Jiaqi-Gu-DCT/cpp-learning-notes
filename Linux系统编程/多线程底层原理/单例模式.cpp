// 请用 thread实现线程安全的懒汉式单例模式，要求： 
// ‑使用双重检查锁定（Double‑Checked Locking, DCL） 
// ‑考虑内存可见性和指令重排序问题
// ‑提供获取实例的接口  ‑写一个多线程测试程序验证正确性
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include <chrono>

class Singleton{
public:
    // 禁止拷贝与移动语义，保证单例的唯一性
    Singleton(const Singleton&) = delete;
    Singleton& operator= (const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator= (Singleton&&) = delete;

    // 对外获取实例的接口
    static Singleton* getInstance();

    // 销毁实例
    static void destroyInstance();

    // 示例成员接口
    int getData() const {return m_data;}
    const std::string& getName() const{return m_name;}

private:
    int m_data;
    std::string m_name;

    // 私有构造函数，禁止外部直接创建
    Singleton() : m_data(42), m_name("景和"){
        std::cout << "[线程" << std::this_thread::get_id() << "]创建了单例实例，地址是" << this << std::endl;
    }
    ~Singleton() = default;

    // 原子指针，解决指令重排序问题
    static std::atomic<Singleton*> m_instance;
    // 保护实例创建的互斥锁
    static std::mutex m_mutex;
};

// 静态成员变量初始化
std::atomic<Singleton*> Singleton::m_instance{nullptr};
std::mutex Singleton::m_mutex;

// 双重检查锁定
Singleton* Singleton::getInstance(){
    // memory_order_acquire：保证读到指针后，能看到实例内部的完整初始化数据
    Singleton* tmp = Singleton::m_instance.load(std::memory_order_acquire);

    // 第一次检查：无锁快速路径
    if(tmp == nullptr){
        // 只有实例为空时，才能进入加锁分支
        std::lock_guard<std::mutex> lk(m_mutex);
        tmp = Singleton::m_instance.load(std::memory_order_acquire);
        // 第二次检查：持锁状态下的二次确认
        // 防止多线程共同创建实例，违背单例模式设计
        if(tmp == nullptr){
            tmp = new Singleton();
            // memory_order_release: 保证实例完全初始化完成后，指针才对其他线程可见
            Singleton::m_instance.store(tmp, std::memory_order_release);
        }
    }
    return tmp;
}

void Singleton::destroyInstance(){
    std::lock_guard<std::mutex> lk(m_mutex);
    Singleton*tmp = Singleton::m_instance.load(std::memory_order_acquire);
    if(tmp != nullptr){
        delete tmp;
        Singleton::m_instance.store(nullptr, std::memory_order_release);
    }
}

// 多线程测试
constexpr int NUM_THREAD = 10;
constexpr int NUM_TIMES = 5;

void threadTask(int id){
    for(int i = 0; i < NUM_TIMES; i++){
        Singleton* s = Singleton::getInstance();
        printf("线程%d，第%d次获取实例：地址%p，值%d，名称%s\n", id, i + 1, s, s->getData(), s->getName().c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

int main(){
    std::cout << "现在开始共计" << NUM_THREAD << "个线程的实例获取。" << std::endl;
    std::vector<std::thread> threads;
    for(int i = 0; i < NUM_THREAD; i++){
        threads.emplace_back(threadTask, i+1);
    }

    for(auto& j : threads){
        j.join();
    }

    std::cout << "所有线程执行完成！\n" << std::endl;
    std::cout << "最终的单例实例位置：" << Singleton::getInstance() << std::endl;

    Singleton::destroyInstance();
    return 0;
}