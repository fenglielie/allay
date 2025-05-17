#pragma once

#if defined(_MSC_VER)
#pragma warning(disable : 4996)

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#endif

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#endif

#ifndef _WIN32
#include <iostream>
#endif

#include <optional>
#include <string>

class WindowsConsole {
public:
    static int init() noexcept {
#ifdef _WIN32
        try {
            SetConsoleOutputCP(65001);
            SetConsoleCP(65001);

            // Set output mode to handle virtual terminal sequences
            // To support ansi colorful output
            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hOut == INVALID_HANDLE_VALUE) { return -1; }

            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode) == 0) { return -1; }

            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
        catch (...) {
            return -1;
        }
#endif

        init_flag = true;
        return 0;
    }

    // Get utf8 input:
    // Use Windows API to read wide characters from the console and convert to
    // UTF-8 on Windows.
    // Use std::getline on Linux, directly.
    static std::optional<std::string> utf8_input() {
        if (!init_flag) { init(); }

#ifdef _WIN32
        HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
        if (hIn == INVALID_HANDLE_VALUE) { return std::nullopt; }

        std::wstring wide_buffer;
        std::wstring temp_buf(128, L'\0');

        while (true) {
            DWORD cnt = 0;
            if ((ReadConsoleW(hIn, temp_buf.data(), 128, &cnt, nullptr) == 0)
                || cnt == 0) {
                break;
            }
            wide_buffer.append(temp_buf, 0, cnt);
            if (cnt < 128) { break; }
        }

        if (wide_buffer.empty()) { return std::nullopt; }

        int utf8_buf_size = static_cast<int>(wide_buffer.size()) * 4;
        std::string utf8_buffer(static_cast<size_t>(utf8_buf_size), '\0');
        BOOL use_default_char = 0;
        int len = WideCharToMultiByte(CP_UTF8, 0, wide_buffer.data(),
                                      static_cast<int>(wide_buffer.size()),
                                      utf8_buffer.data(), utf8_buf_size,
                                      nullptr, &use_default_char);
        if (use_default_char != 0 || len == 0) {
            return std::nullopt;  // Conversion failed
        }

        if (len < 2 || utf8_buffer[static_cast<size_t>(len - 2)] != '\r'
            || utf8_buffer[static_cast<size_t>(len - 1)] != '\n') {
            return std::nullopt;  // Invalid, missing CRLF at the end
        }

        return utf8_buffer.substr(0,
                                  static_cast<size_t>(len - 2));  // Remove CRLF
#else
        std::string input;
        if (!std::getline(std::cin, input)) { return std::nullopt; }
        return input;
#endif
    }

private:
    inline static bool init_flag = false;
};
