#include "allay/progress/progress.hpp"

#include <thread>

#ifdef _WIN32
#include "windows.h"  // IWYU pragma: keep
#endif

void test_pbar(Progress pbar) {
    for (int i = 1; i <= 100; i++) {
        pbar.update(i / 100.0);
        std::this_thread::sleep_for(std::chrono::milliseconds(i / 3));
    }
    Progress::nextline();
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    test_pbar(Progress{});
    test_pbar(Progress{}.enable_color().set_desc("Processing: "));
    test_pbar(Progress{LongFormatter::create(20, 0)});
    test_pbar(Progress{LongFormatter::create(20, 1)});
    test_pbar(Progress{LongFormatter::create(20, 2)});
    test_pbar(Progress{LongFormatter::create(20, 3)});
    test_pbar(Progress{LongFormatter::create(20, 3)}.enable_color());

    return 0;
}
