#include "allay/ini_parser/ini_parser.hpp"

#include <iostream>

int main() {
    auto ini = IniParser{};
    try {
        ini.read(PREFIX + std::string("/config.ini"));
        std::cout << "n: " << ini.get("n").value_or("[x]") << '\n';
        std::cout << "pi: " << ini.get("pi").value_or("[x]") << '\n';
        std::cout << "msg: " << ini.get("msg").value_or("[x]") << '\n';
        std::cout << "symbol: " << ini.get("symbol").value_or("[x]") << '\n';
        std::cout << "settings.enabled: "
                  << ini.get("settings", "enabled").value_or("[x]") << '\n';
        std::cout << "settings.count: "
                  << ini.get("settings", "count").value_or("[x]") << '\n';

        std::cout << "\nexport all\n";

        auto data = ini.export_all();
        for (const auto &section : data) {
            if (!section.first.empty()) {
                std::cout << "[" << section.first << "]" << '\n';
            }
            for (const auto &pair : section.second) {
                std::cout << pair.first << " = " << pair.second << '\n';
            }
        }

        ini.set("new_section", "new_key", "new_value");
        ini.write(PREFIX + std::string("/tmp_config.ini"),
                  std::ios::trunc);
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
    catch (...) {
        std::cout << "unknown exception\n";
    }

    return 0;
}
