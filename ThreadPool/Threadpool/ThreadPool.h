#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <type_traits>
#include <utility>

class ThreadPool{
public:
    // 构造函数：启动numThreads个线程
    explicit ThreadPool(size_t numThreads);

    // 析构函数：停止所有线程，等待任务完成
    ~ThreadPool();

    // 禁止拷贝：线程池不应该被复制
    // 1 拷贝构造
    ThreadPool(const ThreadPool&) = delete;
    // 2 等号运算符重载
    ThreadPool& operator= (const ThreadPool&) = delete;

    // 提交任务：支持任意可调用对象和参数，返回std::future
    template<typename Func, typename... Args>
    auto submit(Func&& func, Args&&... args) -> std::future<typename std::invoke_result_t<Func, Args...>>;

    // 获取当前工作线程数量
    size_t workerCount() const{
        return m_workers.size();
    }

    // 获取等待执行的任务数量
    size_t pendingTaskCount() const;

private:
friend void testSubmitAfterStop(ThreadPool& pool);
    // 工作线程的主循环函数
    void workerLoop();

    // 从任务队列中取出一个任务（阻塞直到有任务或停止）
    bool popTask(std::function<void()>& task);

    // 停止所有线程
    void stop();

    mutable std::mutex m_mutex; // 保护共享数据
    std::condition_variable m_condition; // 唤醒工作线程
    std::queue<std::function<void()>> m_tasks; // 任务队列
    std::vector<std::thread> m_workers; // 工作线程集合
    bool m_stop = false; // 停止标志
};

// 有参构造
inline ThreadPool::ThreadPool(size_t numThreads){
    for(size_t i = 0; i < numThreads; i++){
        m_workers.emplace_back(&ThreadPool::workerLoop, this);
    }
}

// 析构函数
inline ThreadPool::~ThreadPool(){
    stop();
}

// 停止
inline void ThreadPool::stop(){
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_stop) return;
        m_stop = true;
    }

    m_condition.notify_all();

    for(auto& worker : m_workers){
        if(worker.joinable()){
            worker.join();
        }
    }
}

// 主循环函数
inline void ThreadPool::workerLoop(){
    while(true){
        // 万能函数体：返回值void，参数列表（）为空，充当中转站
        std::function<void()> task;
        bool hasTask = popTask(task);

        if(!hasTask){
            break;
        }
        task();
    }
}

// 取出一个任务
inline bool ThreadPool::popTask(std::function<void()>& task){
    std::unique_lock<std::mutex> lock(m_mutex);

    m_condition.wait(lock, [this](){
        return m_stop || !m_tasks.empty();
    });

    if(m_stop && m_tasks.empty()){
        return false;
    }

    task = std::move(m_tasks.front());
    m_tasks.pop();
    return true;
}

// 获取等待任务的数量
inline size_t ThreadPool::pendingTaskCount() const{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_tasks.size();
}

// 任务的提交 submit
// 尾置返回类型，auto只是一个占位符
template<typename Func, typename... Args>
auto ThreadPool::submit(Func&& func, Args&&... args) -> std::future<typename std::invoke_result_t<Func, Args...>>{
    using ReturnType = typename std::invoke_result_t<Func, Args...>;
    // 把用户的函数和参数“打包”成一个可在任意线程执行、并能异步取回结果的任务包。
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    auto future = task -> get_future();
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_stop){
            throw std::runtime_error("ThreadPool: submit on stopped ThreadPool");
        }
        m_tasks.emplace([task](){(*task)();});
    }
    m_condition.notify_one();
    return future;
}

#endif