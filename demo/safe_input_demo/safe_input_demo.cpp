#include "allay/safe_input/safe_input.hpp"

int main(int argc, char *argv[]) {
    auto s = SafeInput().get<int>("Please input a int in (0,5): ",
                                  [](int p) { return 0 < p && p < 5; });
    std::cout << "s=" << s << '\n';

    auto d = SafeInput().get<double>("Please input a double: ", true);
    std::cout << "final: d=" << d << '\n';

    auto p = SafeInput().get<bool>("input a bool: ", true);
    std::cout << "final: p=" << p << '\n';

    SafeInput().confirm("Continue or exit?");

    SafeInput().pause();

    return 0;
}
