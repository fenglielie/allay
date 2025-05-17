#pragma once

#include <algorithm>
#include <cstddef>
#include <source_location>
#include <stack>
#include <string>

class MTracer {
public:
    // NOLINTNEXTLINE(fuchsia-default-arguments-declarations)
    explicit MTracer(const std::source_location &location =
                         std::source_location::current()) {
        get_stack().push(location);
    }

    ~MTracer() { get_stack().pop(); }

    MTracer &operator=(const MTracer &) = delete;
    MTracer(const MTracer &) = delete;

    [[nodiscard]] static std::string dump_string() {
        std::stack<std::source_location> tracer_stack(get_stack());

        std::string str_result = "\nCall stack (MTRACER):";

        std::size_t i = tracer_stack.size();
        while (!tracer_stack.empty()) {
            std::string str_tmp = '\n' + std::to_string(i - 1) + ": "
                                  + format(tracer_stack.top());
            std::ranges::replace(str_tmp, '\\', '/');
            str_result += str_tmp;
            tracer_stack.pop();
            --i;
        }

        return str_result + '\n';
    }

    static std::string format(const std::source_location &location) {
        return std::string{location.file_name()} + ":"
               + std::to_string(location.line()) + " "
               + location.function_name();
    }

private:
    static std::stack<std::source_location> &get_stack() {
        static std::stack<std::source_location> m_tracer_stack;
        return m_tracer_stack;
    }
};

#ifdef USE_MTRACER

#define MTRACER_CB_INNER(a, b) a##b
#define MTRACER_CB(a, b) MTRACER_CB_INNER(a, b)

#define MTRACER MTracer MTRACER_CB(tmp_tracer_, __LINE__)
#define MTRACER_DUMP_STRING MTracer::dump_string()
#define MTRACER_DUMP                                                           \
    do { std::cerr << MTracer::dump_string(); } while (false)

#else

#define MTRACER ((void)0)
#define MTRACER_DUMP_STRING                                                    \
    std::string("\nMTracer: Define USE_MTRACER to use MTracer.\n")
#define MTRACER_DUMP ((void)0)

#endif
