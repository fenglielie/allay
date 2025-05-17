#include "allay/mtimer/mtimer.hpp"

#include <thread>

void example_function() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main() {
    {
        ScopedMTimer timer("Main block");
        example_function();
    }

    {
        MTimer timer;
        timer.start();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        timer.stop();
        std::cout << std::format("Manual timing: {} ms\n", timer.elapsed_ms());
    }
    {
        MTimer timer;
        timer.start();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // no stop() return 0
        std::cout << std::format("(no stop) Manual timing: {} ms\n", timer.elapsed_ms());
    }
    {
        MTimer timer;
        // no start() return 0
        std::this_thread::sleep_for(std::chrono::seconds(1));
        timer.stop();
        std::cout << std::format("(no start) Manual timing: {} ms\n", timer.elapsed_ms());
    }
    return 0;
}
