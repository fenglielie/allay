#pragma once

#include <chrono>
#include <format>
#include <iostream>
#include <string>

class MTimer {
private:
    std::chrono::high_resolution_clock::time_point m_start_time;
    std::chrono::high_resolution_clock::time_point m_end_time;

public:
    void start() { m_start_time = std::chrono::high_resolution_clock::now(); }

    void stop() { m_end_time = std::chrono::high_resolution_clock::now(); }

    double elapsed_ns() const { return get_elapsed_time<std::nano>(); }

    double elapsed_us() const { return get_elapsed_time<std::micro>(); }

    double elapsed_ms() const { return get_elapsed_time<std::milli>(); }

    double elapsed_s() const { return get_elapsed_time<std::ratio<1>>(); }

private:
    template <typename T>
    double get_elapsed_time() const {
        if (m_start_time == std::chrono::high_resolution_clock::time_point()
            || m_end_time == std::chrono::high_resolution_clock::time_point()
            || m_end_time < m_start_time) {
            return 0.0;
        }

        return std::chrono::duration<double, T>(m_end_time - m_start_time)
            .count();
    }
};

class ScopedMTimer {
private:
    MTimer m_timer;
    std::string m_name;

public:
    explicit ScopedMTimer(std::string name) : m_name(std::move(name)) {
        m_timer.start();
    }

    ScopedMTimer() : ScopedMTimer("ScopedMTimer") {}

    ~ScopedMTimer() {
        m_timer.stop();

        double ns = m_timer.elapsed_ns();
        std::string output;

        if (ns < 1e3) {
            output = std::format("{} elapsed: {} ns\n", m_name, ns);
        }
        else if (ns < 1e6) {
            output = std::format("{} elapsed: {} us\n", m_name,
                                 m_timer.elapsed_us());
        }
        else if (ns < 1e9) {
            output = std::format("{} elapsed: {} ms\n", m_name,
                                 m_timer.elapsed_ms());
        }
        else {
            output =
                std::format("{} elapsed: {} s\n", m_name, m_timer.elapsed_s());
        }

        std::cout << output;
    }

    ScopedMTimer(const ScopedMTimer &) = delete;
    ScopedMTimer &operator=(const ScopedMTimer &) = delete;
};
