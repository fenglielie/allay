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
        return 0;
    }

    // Use Windows API to read wide characters from the console and convert to
    // UTF-8.
    static std::optional<std::string> utf8_input() {
#ifdef _WIN32
        HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
        if (hIn == INVALID_HANDLE_VALUE) { return std::nullopt; }

        std::wstring wide_buffer;
        std::wstring temp_buf(128, L'\0');  // 小块缓冲区读取

        while (true) {
            DWORD chars_read = 0;
            if ((ReadConsoleW(hIn, temp_buf.data(), 128, &chars_read, nullptr)
                 == 0)
                || chars_read == 0) {
                break;
            }
            wide_buffer.append(temp_buf, 0, chars_read);
            if (chars_read < 128) {  // 说明输入结束
                break;
            }
        }

        if (wide_buffer.empty()) { return std::nullopt; }

        int utf8_buf_size = static_cast<int>(wide_buffer.size()) * 4;
        std::string utf8_buffer(utf8_buf_size, '\0');
        BOOL use_default_char = 0;
        int len = WideCharToMultiByte(CP_UTF8, 0, wide_buffer.data(),
                                      static_cast<int>(wide_buffer.size()),
                                      utf8_buffer.data(), utf8_buf_size,
                                      nullptr, &use_default_char);
        if (use_default_char != 0 || len == 0) {  // Conversion failed
            return std::nullopt;
        }

        if (len < 2 || utf8_buffer[len - 2] != '\r'
            || utf8_buffer[len - 1] != '\n') {
            // Invalid result, missing CRLF at the end
            return std::nullopt;
        }

        return utf8_buffer.substr(0, len - 2);  // Remove CRLF
#else
        std::string input;
        if (!std::getline(std::cin, input)) { return std::nullopt; }
        return input;
#endif
    }
};
