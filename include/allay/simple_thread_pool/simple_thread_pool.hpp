#pragma once

#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class SimpleThreadPool {
public:
    // 构造时自动开启线程池
    explicit SimpleThreadPool(uint32_t thread_num)
        : m_thread_num(thread_num > 0 ? thread_num : 1) {
        start();
    }

    // 析构时自动关闭线程池
    ~SimpleThreadPool() { stop(); }

    // 禁止复制
    SimpleThreadPool(const SimpleThreadPool &) = delete;
    SimpleThreadPool &operator=(const SimpleThreadPool &) = delete;

    // 提交任务，含参数，返回future
    template <class F, class... Args>
    auto commit(F &&f, Args &&...args) {
        // 返回类型
        using RetType = std::invoke_result_t<F, Args...>;

        // 如果线程池已经停止，直接返回空的future
        if (!m_running.load())
            throw std::runtime_error("ThreadPool is stopped.");

        // 提交任务
        // 对参数进行完美转发，使用lambda表达式打包可调用对象和参数
        // 创建packaged_task对象进一步包装
        auto new_task_ptr = std::make_shared<std::packaged_task<RetType()>>(
            [func = std::forward<F>(f),
             ... args = std::forward<Args>(args)]() mutable {
                return func(std::forward<Args>(args)...);
            });

        std::future<RetType> result = new_task_ptr->get_future();
        {
            std::lock_guard<std::mutex> mtx_guard(m_mtx);
            m_tasks.emplace([new_task_ptr] { (*new_task_ptr)(); });
        }

        m_cv.notify_one();  // 唤醒一个线程来执行任务

        return result;  // 返回future对象
    }

    // 获取当前可用的线程数量
    uint32_t get_idle_thread_num() const { return m_idle_thread_num; }

    // 获取线程池实例的线程数量
    uint32_t get_thread_num() const { return m_thread_num; }

private:
    // 开启线程池
    void start() {
        std::unique_lock<std::mutex> mtx_guard(m_mtx);

        m_running.store(true);
        m_idle_thread_num.store(m_thread_num);

        for (uint32_t i = 0; i < m_thread_num; ++i) {
            // 向线程池中填充默认任务
            m_pool.emplace_back([this]() {  // 必须显式捕获this指针
                while (this->m_running.load()) {  // 线程池开启时无法跳出循环
                    std::packaged_task<void()> task;

                    {
                        // 获取互斥锁
                        std::unique_lock<std::mutex> mtx_guard2(m_mtx);

                        // 通过条件变量让线程陷入等待
                        // 只有当前的任务队列非空或线程池已经被关闭时才会被成功唤醒
                        this->m_cv.wait(mtx_guard2, [this] {
                            return !this->m_running.load()
                                   || !this->m_tasks.empty();
                        });

                        // 如果任务队列为空，代表没有有效任务，直接return
                        // 此时通常意味着线程池被关闭，可以跳出while循环
                        if (this->m_tasks.empty()) { return; }

                        // 以move方式获取队列中的首位任务并执行
                        task = std::move(this->m_tasks.front());
                        this->m_tasks.pop();
                    }

                    this->m_idle_thread_num--;  // 可用线程数-1
                    task();
                    this->m_idle_thread_num++;  // 可用线程数+1
                }
            });
        }
    }

    // 关闭线程池
    void stop() {
        m_running.store(false);  // 设置为停止状态
        m_cv.notify_all();       // 唤醒所有线程

        // 合并所有线程
        for (auto &td : m_pool) {
            if (td.joinable()) { td.join(); }
        }
    }

    std::mutex m_mtx;                        // 互斥锁
    std::condition_variable m_cv;            // 条件变量
    std::atomic_bool m_running;              // 线程池是否正在运行
    std::atomic_uint32_t m_thread_num;       // 线程池大小
    std::atomic_uint32_t m_idle_thread_num;  // 可用的空闲线程数

    std::queue<std::packaged_task<void()>> m_tasks;  // 任务队列
    std::vector<std::thread> m_pool;                 // 线程池
};
