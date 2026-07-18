// 输入输出头文件，用于cout打印结果
#include <iostream>
// 核心异步组件头文件：std::packaged_task / std::future 定义在此
#include <future>
// std::make_shared / std::shared_ptr 智能指针头文件
#include <memory>
// std::bind 绑定函数工具头文件
#include <functional>

// 普通计算函数，接收两个int，返回二者之和
int calc(int a, int b){
    return a + b;
}

int main(){
    // std::invoke_result_t<可调用对象类型, 参数1类型, 参数2类型...>
    // decltype(calc)：获取calc函数本身的类型
    // 后面两个int代表调用calc需要传入2个int参数
    // 整行作用：编译期自动推导出 calc(int,int) 的返回类型 int，别名ReturnType
    using ReturnType = std::invoke_result_t<decltype(calc), int, int>;

    // std::bind(函数, 参数1, 参数2,...)
    // 将calc函数和固定参数7、8绑定，生成一个【无参可调用仿函数】bindTask
    // 后续直接调用 bindTask() 等价于 calc(7,8)
    auto bindTask = std::bind(calc, 7, 8);

    // std::make_shared<类型>(构造参数)
    // <>内写纯类型：std::packaged_task<ReturnType()>
    // ()内是构造这个类型需要传入的参数 bindTask
    // std::packaged_task<ReturnType()> 含义：包装一个【无参、返回值为ReturnType(int)】的任务
    // make_shared：在堆上分配packaged_task对象，返回shared_ptr共享智能指针
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(bindTask);

    // task 是 std::shared_ptr 智能指针，不是packaged_task本体对象
    // 不能直接写 task(); 会报编译错误
    // *task 解引用智能指针，拿到堆上真实的packaged_task对象，调用operator()执行任务
    // 执行后calc(7,8)计算出15，结果存入packaged_task内部的共享状态
    (*task)();

    // 1. task是shared_ptr，访问内部成员函数必须用箭头 ->
    //    错误写法：task.get_future() ，智能指针没有get_future方法
    // 2. get_future() 获取和任务绑定的std::future对象，作为结果通道
    //    .get() 阻塞等待任务执行完成，取出函数返回值；任务已执行完则直接返回结果
    int result = task->get_future().get();

    // 打印最终计算结果
    std::cout << "Result:" << result << std::endl;
    return 0;
}
