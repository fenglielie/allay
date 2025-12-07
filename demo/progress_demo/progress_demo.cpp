#include "allay/progress/progress.hpp"

#include <thread>

#ifdef _WIN32
#include "windows.h"  // IWYU pragma: keep
#endif

void test_pbar(Progress pbar) {
    for (int i = 1; i <= 100; i++) {
        pbar.update(i / 100.0);
        std::this_thread::sleep_for(std::chrono::milliseconds(i / 2));
    }
    Progress::nextline();
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    { test_pbar(Progress{}); }
    {
        auto st = ProgressStyle::Classic();
        st.color_of_pct = ColorGradient{{0, 0, 255}};
        st.color_of_bar = ColorGradient::Ocean();
        st.color_of_time = ColorGradient::Energy();
        test_pbar(Progress{st});
    }
    {
        auto st = ProgressStyle::Block();
        st.color_of_pct = ColorGradient{{0, 255, 0}};
        st.color_of_bar = ColorGradient::Energy();
        test_pbar(Progress{st});
    }
    {
        auto st = ProgressStyle::Braille();
        st.color_of_pct = ColorGradient{{0, 255, 0}};
        st.color_of_bar = ColorGradient::Heat();
        test_pbar(Progress{st});
    }
    return 0;
}
