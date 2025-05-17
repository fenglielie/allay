#include "allay/simple_thread_pool/simple_thread_pool.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

int main() {
    try {
        // 获取线程池实例
        auto pool = SimpleThreadPool{5};

        // 提交一些任务
        std::vector<std::future<int>> results;
        results.reserve(10);

        for (int i = 0; i < 10; ++i) {
            std::string msg = "current available threads: "
                              + std::to_string(pool.get_idle_thread_num())
                              + '\n';
            std::cout << msg;

            // 提交任务
            if (i % 3 == 0) {
                auto func = [i]() -> int {
                    std::string msg2 =
                        "Task " + std::to_string(i) + " executed\n";
                    std::cout << msg2;
                    std::this_thread::sleep_for(std::chrono::seconds(3 + i));

                    throw std::runtime_error("Exception in task "
                                             + std::to_string(i));
                };
                results.emplace_back(pool.commit(func));
            }
            else {
                auto func = [i](int j) -> int {
                    std::string msg2 =
                        "Task " + std::to_string(i) + " executed\n";
                    std::cout << msg2;
                    std::this_thread::sleep_for(std::chrono::seconds(3 + i));
                    return i * j;
                };
                results.emplace_back(pool.commit(func, i + 1));
            }
        }

        // 获取任务结果
        for (size_t i = 0; i < results.size(); ++i) {
            try {
                std::string msg = "Result of task " + std::to_string(i) + ": "
                                  + std::to_string(results[i].get()) + '\n';
                std::cout << msg;
            }
            catch (const std::exception &e) {
                std::cerr << "Exception caught: " << e.what() << '\n';
            }
            catch (...) {
                std::cerr << "Unknown exception caught\n";
            }
        }
    }
    catch (const std::exception &e) {
        std::cerr << "Exception caught: " << e.what() << '\n';
    }

    return 0;
}
