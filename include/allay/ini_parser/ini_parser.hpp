#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

class IniParser {
public:
    void set(const std::string &section, const std::string &key,
             const std::string &value) {
        m_data[section][key] = value;
    }

    void set(const std::string &key, const std::string &value) {
        m_data[""][key] = value;  // default section
    }

    auto get(const std::string &section,
             const std::string &key) const -> std::optional<std::string> {
        auto iter = m_data.find(section);
        if (iter != m_data.end()) {
            auto itKey = iter->second.find(key);
            if (itKey != iter->second.end()) { return itKey->second; }
        }
        return std::nullopt;
    }

    auto get(const std::string &key) const {
        return get("", key);  // default section
    }

    void read(const std::string &file_name) {
        std::fstream f(file_name, std::ios::in);
        if (f.fail()) {
            throw std::runtime_error{"file open error: " + file_name};
        }

        std::string line;
        std::string cur_section;
        while (std::getline(f, line)) {
            clean_str(line);

            // support multiline
            while (!line.empty() && line.back() == '\\') {
                line.pop_back();
                std::string next_line;
                if (std::getline(f, next_line)) {
                    clean_str(next_line);
                    line += next_line;
                }
            }

            // ':' -> '='
            std::size_t n_colon_start = line.find_first_of(':');
            if (n_colon_start != std::string::npos) {
                line[n_colon_start] = '=';
            }

            // erase comments
            std::size_t n_comment_start = line.find_first_of('#');
            if (n_comment_start != std::string::npos) {
                line.erase(n_comment_start);
            }

            trim_str(line, " ");

            if (line.empty()) continue;

            // switch section
            if (line.front() == '[' && line.back() == ']') {
                cur_section = line.substr(1, line.size() - 2);
                trim_str(cur_section, " ");
                continue;
            }

            // add pair
            if (!add_pair(cur_section, line)) {
                f.close();
                throw std::runtime_error{"line parse error: " + line};
            }
        }
    }

    void write(const std::string &file_name,
               const std::ios_base::openmode mode) const {
        std::fstream f(file_name, std::ios::out | mode);  // NOLINT
        if (f.fail()) {
            throw std::runtime_error{"file open error: " + file_name};
        }

        for (const auto &section : m_data) {
            if (!section.first.empty()) {
                f << "\n[" << section.first << "]\n";
            }
            for (const auto &pair : section.second) {
                f << pair.first << " = " << pair.second << '\n';
            }
        }

        f.close();
    }

    auto export_all() const { return m_data; }

private:
    std::map<std::string, std::map<std::string, std::string>> m_data;

    static void split_str(const std::string &s,
                          std::vector<std::string> &tokens,
                          const std::string &delimiters) {
        auto last_pos = s.find_first_not_of(delimiters, 0);
        auto pos = s.find_first_of(delimiters, last_pos);
        while (std::string::npos != pos || std::string::npos != last_pos) {
            tokens.emplace_back(s.substr(last_pos, pos - last_pos));
            last_pos = s.find_first_not_of(delimiters, pos);
            pos = s.find_first_of(delimiters, last_pos);
        }
    }

    static void clean_str(std::string &str) {
        std::ranges::replace(str, '\n', ' ');
        std::ranges::replace(str, '\r', ' ');
        std::ranges::replace(str, '\t', ' ');
    }

    static void trim_str(std::string &s, const std::string &delimiters) {
        if (s.empty()) return;

        s.erase(0, s.find_first_not_of(delimiters));
        s.erase(s.find_last_not_of(delimiters) + 1);
    }

    bool add_pair(const std::string &cur_section, const std::string &line) {
        std::vector<std::string> str_buff;

        split_str(line, str_buff, "=");

        if (str_buff.size() == 2) {
            std::string key = str_buff[0];
            trim_str(key, " ");

            std::string value = str_buff[1];
            trim_str(value, " ");

            m_data[cur_section][key] = value;
            return true;
        }

        if (str_buff.size() == 1) {
            std::string key = str_buff[0];
            trim_str(key, " ");

            m_data[cur_section][key] = "";  // value = ""
            return true;
        }

        return false;
    }
};
