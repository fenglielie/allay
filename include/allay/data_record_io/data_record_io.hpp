#pragma once

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

template <typename... Ts>
class DataRecordIO {
public:
    using Record = std::tuple<Ts...>;

    static std::vector<Record> read(std::istream &is, char delimiter) {
        std::vector<Record> results;
        std::string line;
        while (std::getline(is, line)) {
            clean_line(line);
            if (!line.empty()) {
                results.emplace_back(parse_record(line, delimiter));
            }
        }
        return results;
    }

    static void write(std::ostream &os, const std::vector<Record> &data,
                      char delimiter) {
        for (const auto &record : data) { write_record(os, record, delimiter); }
    }

private:
    static void clean_line(std::string &line) {
        // Replace CR/LF/TAB with space
        std::ranges::replace(line, '\r', ' ');
        std::ranges::replace(line, '\t', ' ');

        // Trim head
        line.erase(0, line.find_first_not_of(' '));

        // Trim tail
        auto pos = line.find_last_not_of(' ');
        if (pos != std::string::npos) { line.erase(pos + 1); }

        // Remove inline comment '#'
        if (auto idx = line.find('#'); idx != std::string::npos) {
            line.erase(idx);
        }

        // Final trim again
        line.erase(0, line.find_first_not_of(' '));
        pos = line.find_last_not_of(' ');
        if (pos != std::string::npos) { line.erase(pos + 1); }
    }

    static Record parse_record(const std::string &s, char delimiter) {
        std::istringstream ss(s);
        Record r;
        read_tuple(ss, r, delimiter);
        return r;
    }

    template <std::size_t I = 0, typename... Args>
    static void read_tuple(std::istringstream &ss, std::tuple<Args...> &t,
                           char delimiter) {
        if constexpr (I < sizeof...(Args)) {
            if (!(ss >> std::get<I>(t))) {
                throw std::runtime_error("Failed to parse field at index "
                                         + std::to_string(I));
            }

            if constexpr (I + 1 < sizeof...(Args)) {
                if (delimiter != ' ') {
                    char sep{0};
                    if (!(ss >> sep) || sep != delimiter) {
                        throw std::runtime_error(
                            "Missing delimiter '" + std::string(1, delimiter)
                            + "' after field index " + std::to_string(I));
                    }
                }
            }

            read_tuple<I + 1, Args...>(ss, t, delimiter);
        }
    }

    template <std::size_t I = 0, typename... Args>
    static void write_tuple(std::ostream &os, const std::tuple<Args...> &t,
                            char delimiter) {
        if constexpr (I < sizeof...(Args)) {
            os << std::get<I>(t);

            if constexpr (I + 1 < sizeof...(Args)) { os << delimiter; }
            else { os << '\n'; }

            write_tuple<I + 1, Args...>(os, t, delimiter);
        }
    }

    static void write_record(std::ostream &os, const Record &r,
                             char delimiter) {
        write_tuple(os, r, delimiter);
    }
};
