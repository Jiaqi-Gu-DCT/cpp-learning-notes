#include "ThreadPool.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <string>

using namespace std::chrono_literals;

// 辅助函数 & 工具

// 模拟耗时的乘方计算
int slowSquare(int x){
    std::this_thread::sleep_for(100ms);
    return x * x;
}

// 模拟耗时的无返回值任务
void slowPrint(const std::string& msg){
    std::this_thread::sleep_for(50ms);
    std::cout << msg << std::endl;
}

// 测试1：无返回值的任务，验证其是否能正常执行
void testVoidTask(ThreadPool& pool){
    std::cout << "\n ---测试1：无返回值任务---" << std::endl;
    for(int i = 0; i < 5; i++){
        pool.submit([i](){
            std::cout << "Task" << i << "is working in the thread" << std::this_thread::get_id() <<std::endl;
        });
    }
    std::this_thread::sleep_for(200ms);
}

// 测试2：有返回值的任务，验证future的机制
void testReturnTask(ThreadPool& pool){
    std::cout << "\n ---测试2：有返回值任务---" << std::endl;
    std::vector<std::future<int>> futures;
    for(int i = 1; i <= 5; i++){
        auto fut = pool.submit(slowSquare, i);
        futures.push_back(std::move(fut));
    }
    for(size_t i = 0; i < futures.size(); i++){
        std::cout << "slowSquare(" << i+1 << ")=" << futures[i].get() << std::endl;
    }
}

// 测试3：批量任务，验证队列积压与线程并发执行
void testBulkTasks(ThreadPool& pool){
    std::cout << "\n ---测试3：批量耗时任务---" << std::endl;
    std::atomic<int> counter{0};
    for(int i = 0; i < 20; i++){
        pool.submit([&counter, i](){
            std::this_thread::sleep_for(50ms);
            counter++;
        });
    }
    std::cout << "已提交了20项任务，当前线程等待任务数量为：" << pool.pendingTaskCount() << std::endl;
    std::this_thread::sleep_for(600ms);
    std::cout << "任务执行完毕，共计执行：" << counter.load() << "个任务。" << std::endl;
}

// 测试4：异常安全，验证异常是否会导致线程崩溃
void testException(ThreadPool& pool){
    std::cout << "\n ---测试4：异常安全测试---" << std::endl;

    // 4.1 抛出异常任务
    auto fut1 = pool.submit([]() -> int{
        std::cout << "抛出了 runtime_error" << std::endl;
        throw std::runtime_error("任务内部异常！");
        return 0;
    });

    try{
        fut1.get();
    }catch(const std::exception& exp){
        std::cout << "成功捕获异常：" << exp.what() << std::endl;
    }

    // 4.2 验证线程池未受到影响
    auto fut2 = pool.submit([](){return 42;});
    std::cout << "异常后任务正常执行，返回值为：" << fut2.get() << std::endl;
}

// 测试5：移动语义，验证shared_ptr和移动构造的正确性
void testMoveSemantic(ThreadPool& pool){
    std::cout << "\n ---测试5：语义转发测试---" << std::endl;
    std::unique_ptr<int> ptr = std::make_unique<int>(100);
    auto fut = pool.submit([ptr = std::move(ptr)]() -> int{
        return *ptr;
    });

    std::cout << "移动语义执行结果：" << fut.get() << std::endl;

}

// 测试6：高并发压力测试，验证锁竞争和线程安全
void testHighLoad(ThreadPool& pool){
    std::cout << "\n ---测试6：高并发压力测试---" << std::endl;
    std::atomic<int> counter{0};
    const int TASK = 100;

    auto start = std::chrono::steady_clock::now();
    for(int i = 0; i < TASK; i++){
        pool.submit([&counter](){
            volatile int x = 0;
            for(int j = 0; j < 1000; j++) x += j;
            counter++;
        });
    }

    while(pool.pendingTaskCount() > 0){
        std::this_thread::sleep_for(10ms);
    }
    std::this_thread::sleep_for(50ms);

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "提交了：" << TASK << "个任务，   完成了：" << counter.load() << "个任务。" << std::endl;
    std::cout << "耗时：" << duration << "毫秒！" << std::endl;
}

// 测试7：停止后提交任务，验证优雅之关闭
void testSubmitAfterStop(ThreadPool& pool){
    std::cout << "\n ---测试7：优雅关闭测试---" << std::endl;
    pool.stop();

    try{
        pool.submit([](){
            return 42;
        });
        std::cout << "测试失败！停止后应该抛出异常！" << std::endl;
    }catch(const std::runtime_error& e){
        std::cout << "测试成功！捕获到异常：" << e.what() << std::endl;
    }
}

// 测试8：future超时与取消，验证get的阻塞特性
// 当异步任务尚未执行完成时，调用 .get()会挂起当前线程，直到任务执行完毕、结果就绪才会返回
void testFutureTimeoutAndCancel(ThreadPool& pool){
    std::cout << "\n ---测试8：future特性测试---" << std::endl;

    // 8.1 验证超时特性
    std::cout << "\n --- 8.1 超时等待测试 ---" << std::endl;
    auto future_slow = pool.submit([](){
        std::this_thread::sleep_for(200ms);
        return 999;
    });

    std::future_status status = future_slow.wait_for(50ms);
    if(status == std::future_status::timeout){
        std::cout << "50毫秒等待超时，任务尚未完成！" << std::endl;
    }else{
        std::cout << "错误！预期超时但实际未超时！" << std::endl;
    }

    // 8.2 验证任务就绪后的正常获取
    std::cout << "\n --- 8.2 就绪后获取测试 ---" << std::endl;
    auto start = std::chrono::steady_clock::now();
    status = future_slow.wait_for(200ms);
    auto wait_total = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

    if(status == std::future_status::ready){
        // get()只能调用一次，调用后future就会失效，再次调用会抛出异常
        int result = future_slow.get();
        std::cout << "任务已就绪，获取到结果：" << result << "二次等待耗时：" << wait_total << "毫秒！" << std::endl;
    }else{
        std::cout << "发生错误！任务未就绪！" << std::endl;
    }

    // 8.3 协作式任务取消测试
    std::cout << "\n --- 8.3 协作式任务取消测试 ---" << std::endl;
    std::atomic<bool> cancel_flag{false};
    auto fut_cancel = pool.submit([&cancel_flag]() -> int{
        for(int i = 0; i < 100; i++){
            if(cancel_flag.load()){
                std::cout << "接收到取消信号，提前退出" << std::endl;
                return -1;
            }
            std::this_thread::sleep_for(10ms);
        }
        return 0;
    });

    std::this_thread::sleep_for(100ms);
    cancel_flag.store(true);
    std::cout << "已发送取消信号" << std::endl;

    int cancel_result = fut_cancel.get();
    if(cancel_result == -1){
        std::cout << "取消成功，返回正确的取消标记值为：" << cancel_result << std::endl;
    }else{
        std::cout << "取消失败！！！ 返回值为：" << cancel_result << std::endl;
    }
}

// 测试9：pendingTaskCount测试
void testPendingCount(ThreadPool& pool){
    std::cout << "\n ---测试9：PendingCount测试---" << std::endl;
    const int TASK_NUM = 8;
    for(int i = 0; i < TASK_NUM; i++){
        pool.submit([i](){
            std::this_thread::sleep_for(300ms);
        });
    }

    size_t pending = pool.pendingTaskCount();
    std::cout << "提交了：" << TASK_NUM << "个任务，现有：" << pending << "个任务在等待处理。" << std::endl;

    std::this_thread::sleep_for(200ms);
    pending = pool.pendingTaskCount();
    std::cout << "等待200ms后，仍有：" << pending << "个任务在等待处理。" << std::endl;
}

// 测试10：不同类型任务的混合提交
void testMixedTasks(ThreadPool& pool){
    std::cout << "\n ---测试10：混合任务提交测试---" << std::endl;

    auto fut1 = pool.submit(slowSquare,3);

    int factor = 3;
    auto fut2 = pool.submit([factor](int x) -> int{return factor * x;}, 4);

    struct Multiplier{
        int operator()(int x) const{
            return x * 5;
        }
    };
    auto fut3 = pool.submit(Multiplier{}, 6);

    std::cout << "第1个任务返回值：" << fut1.get() << std::endl;
    std::cout << "第2个任务返回值：" << fut2.get() << std::endl;
    std::cout << "第3个任务返回值：" << fut3.get() << std::endl;
}

// 主函数
int main(){
    std::cout << "线程池 v1.0 完整综合测试" << std::endl;
    ThreadPool pool(3);
    std::cout << "\n线程池已创建，工作线程数: " << pool.workerCount() << std::endl;

    // 依次执行所有测试
    testVoidTask(pool);
    testReturnTask(pool);
    testBulkTasks(pool);
    testException(pool);
    testMoveSemantic(pool);
    testHighLoad(pool);
    testFutureTimeoutAndCancel(pool);
    testPendingCount(pool);
    testMixedTasks(pool);
    testSubmitAfterStop(pool);
    
    // 测试析构前的最终状态
    std::cout << "\n=== 最终状态 ===" << std::endl;
    std::cout << "当前等待任务数: " << pool.pendingTaskCount() << std::endl;
    std::cout << "所有测试完成!" << std::endl;
    
    return 0;

}