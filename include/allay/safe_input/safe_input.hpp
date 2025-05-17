#pragma once

#include <iostream>
#include <limits>
#include <string>

class SafeInput {
public:
    SafeInput() : m_in(std::cin), m_out(std::cout) {}

    explicit SafeInput(std::istream &in) : m_in(in), m_out(std::cout) {}

    SafeInput(std::istream &in, std::ostream &out) : m_in(in), m_out(out) {}

    template <typename T, typename CheckerType>
    T get(const std::string &msg, CheckerType checker, bool confirm) const {
        return get_kernel<T>(msg, checker, confirm);
    }

    template <typename T>
    T get(const std::string &msg, bool confirm) const {
        return get_kernel<T>(msg, [](T &) { return true; }, confirm);
    }

    template <typename T, typename CheckerType>
    T get(const std::string &msg, CheckerType checker) const {
        return get_kernel<T>(msg, checker, false);
    }

    template <typename T>
    T get(const std::string &msg) const {
        return get_kernel<T>(msg, [](T &) { return true; }, false);
    }

    bool confirm(const std::string &msg) const {
        bool status = false;
        auto checker = [&](char s) {
            if (is_char(s, 'Y', 'y')) {
                status = true;
                return true;
            }
            if (is_char(s, 'N', 'n')) {
                status = false;
                return true;
            }
            if (is_char(s, 'Q', 'q')) {
                out("quit by SafeInput.");
                exit(0);
            }

            return false;
        };

        get_kernel<char>(msg + " (Y|N|Q): ", checker, false);
        return status;
    }

    bool confirm() const { return confirm("Confirm ?"); }

    void pause() const {
        out("Press enter to continue. ");
        clear_buffer();
    }

private:
    std::istream &m_in;
    std::ostream &m_out;

    std::ostream &out(const std::string &msg) const { return m_out << msg; }

    static bool is_char(char s, char upper, char lower) {
        return (s == upper) || (s == lower);
    }

    void clear_buffer() const {
        m_in.clear();
        m_in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    template <typename T>
    bool in(T &receiver) const {
        m_in >> receiver;
        bool s = m_in.good();

        clear_buffer();
        return s;
    }

    template <typename T, typename CheckerType>
    T get_kernel(const std::string &msg, CheckerType checker,
                 bool confirm) const {
        check_type<T>();
        auto confirmer = [&](char s) {
            if (is_char(s, 'Y', 'y')) { return true; }
            if (is_char(s, 'Q', 'q')) {
                out("quit by SafeInput.");
                exit(0);
            }
            return false;
        };
        T receiver;
        char tmp = 0;

        while (true) {
            out(msg);

            if (in(receiver) && checker(receiver)) {  // pass from checking
                if (!confirm) break;                  // without confirmation

                out("Get: ") << receiver << '\n';
                out("Confirm it? (Y|N|Q): ");

                if (in(tmp) && confirmer(tmp)) break;  // after confirmation
            }

            if (msg.empty()) out("Please input again.\n");
        }
        return receiver;
    }

    template <typename T1, typename T2>
    static constexpr bool Is = std::is_same_v<T1, T2>;

    template <typename T>
    static void check_type() {
        if constexpr (Is<T, std::string> || Is<T, bool> || Is<T, char>
                      || Is<T, int> || Is<T, double> || Is<T, size_t>) {}
        else { static_assert(Is<T, std::string>, "unsupported type"); }
    }
};
