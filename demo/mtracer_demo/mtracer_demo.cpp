#define USE_MTRACER

#include "allay/mtracer/mtracer.hpp"
#include <iostream>

class A {
public:
    static void Fun() {
        MTRACER;
        std::cerr << MTRACER_DUMP_STRING;
    }

    static void Fun2() {
        MTRACER;

        [[maybe_unused]] double value = [](double max_value) {
            double s = 0;
            int n = 1;
            while (s < max_value) {
                s += (1.0 / n);
                ++n;
            }
            return s;
        }(20);

        Fun();
    }

    static void Fun3() {
        MTRACER;

        [[maybe_unused]] double value = [](double max_value) {
            double s = 0;
            int n = 1;
            while (s < max_value) {
                s += (1.0 / n);
                ++n;
            }
            return s;
        }(40);

        Fun2();
    }
};

int main() {
    MTRACER;

    A::Fun3();

    return 0;
}
