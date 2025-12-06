#include "allay/cmd_parser/cmd_parser.hpp"

int main(int argc, char *argv[]) {
    auto parser = CmdParser{};

    parser.add_flag({"--gzip", "-g"}, "use gzip");
    parser.add_flag({"-v", "--verbose"}, "");

    parser.add_option<int>({"-l", "--len"},
                           "option with default value and checker", false, 10,
                           [](int arg) { return arg >= 0; });

    parser.add_option({"-n", "--num"}, "option with default value", false, 0);

    parser.add_option<double>("--scale", "option with checker", true,
                              [](double arg) { return arg >= 0; });

    parser.add_option<int>({"-w", "--weight"},
                           "option without default value and checker", false);

    parser.add_flag({"--help", "-h"}, "print help message", [&parser]() {
        parser.print_usage();
        exit(0);
    });

    // parser.enable_strict_mode();
    parser.parse_check(argc, argv);

    // if (!parser.parse(argc, argv)) {
    //     parser.print_usage();
    //     return 1;
    // }

    auto rest = parser.get_rest();
    std::cout << "Rest:\n";
    for (const auto &s : rest) { std::cout << s << '\n'; }

    std::cout << "Scale: " << parser.get_option<double>("--scale").value()
              << '\n';
    std::cout << "Len: " << parser.get_option<double>("--len").value_or(0)
              << '\n';

    if (auto weights = parser.get_option_all<int>("-w")) {
        std::cout << "Weight: ";
        for (const auto &w : *weights) { std::cout << w << ' '; }
        std::cout << '\n';
    }

    if (auto n = parser.get_count("--gzip")) {
        std::cout << "gzip count: " << n.value() << '\n';
    }

    if (auto n = parser.get_count("--verbose")) {
        std::cout << "verbose count: " << n.value() << '\n';
    }

    return 0;
}
