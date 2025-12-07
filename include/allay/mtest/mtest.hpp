#ifndef MTEST_H_
#define MTEST_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// MTest
class MTest {
public:
    using FuncType = void(int *);

    // test item
    struct TestItem {
        bool match_filter{false};
        bool success{true};  // test result
        FuncType *f;
        const char *itemname;
        const char *fullname;

        TestItem(FuncType *arg_f, const char *arg_itemname,
                 const char *arg_fullname)
            : f(arg_f), itemname(arg_itemname), fullname(arg_fullname) {}
    };

    // test set
    using TestSet = std::vector<TestItem>;

    class MTestMessage {
    public:
        MTestMessage(bool always_output, bool important_output)
            : m_always_output(always_output),
              m_important_output(important_output) {}

        template <typename T>
        MTestMessage &operator<<(T info) {
            if (m_important_output || !brief_output()) {
                if (m_always_output) {
                    std::cout << info;
                    return *this;
                }

                if (!expect_result()) {
                    std::cout << info << '\n';
                    return *this;
                }
            }
            return *this;
        }

        template <typename T>
        static void evaluate_if_required(const char *str, T value) {
            if (std::to_string(value) != str) {
                std::cout << "    Which is: " << value << '\n';
            }
        }

        // record the last EXPECT_XX result.
        static bool &expect_result() noexcept {
            static bool bool_expect_result{true};
            return bool_expect_result;
        }

        // True: only print test failures.
        // False: print more information.(default)
        static bool &brief_output() noexcept {
            static bool bool_expect_result{false};
            return bool_expect_result;
        }

    private:
        const bool m_always_output;     // output even succeed last time.
        const bool m_important_output;  // output even in brief mode.
    };

    static MTestMessage &msg() {
        static MTestMessage msg(true, false);
        return msg;
    }

    static MTestMessage &msg_even_brief() {
        static MTestMessage msg(true, true);
        return msg;
    }

    static MTestMessage &msg_when_fail_even_brief() {
        static MTestMessage msg(false, true);
        return msg;
    }

    static void init_mtest(int argc, char *argv[], const char *main_file) {
        get_instance().set_main_file(main_file);
        get_instance().set_from_argv(argc, argv);
    }

    static void init_mtest(int argc, char *argv[]) {
        get_instance().set_from_argv(argc, argv);
    }

    static void init_mtest(const char *main_file) {
        get_instance().set_main_file(main_file);
    }

    // do nothing
    static void init_mtest() {}

    static int add_test_item(const char *test_set_name,
                             const char *test_itemname, FuncType *f,
                             const char *test_fullname) noexcept {
        get_instance().set_test_item(test_set_name, test_itemname, f,
                                     test_fullname);
        return 0;
    }

    static void set_filter(std::string filter, bool force_modify) {
        if (force_modify || get_instance().m_filter.empty())
            get_instance().set_filter_force(filter);
    }

    // run all tests.
    // return the number of failed test items.
    static int run_all_tests() { return get_instance().run(); }

private:
    static MTest &get_instance() noexcept {
        static MTest entity;
        return entity;
    }

    // run all tests.
    int run() {
        set_filter("*", false);
        set_matched_count();

        show_note_main();
        show_note_filter();

        show_start();

        clock_t start_time_all{0};
        clock_t start_time_test_set{0};
        clock_t start_time_test_item{0};
        clock_t end_time_all{0};
        clock_t end_time_test_set{0};
        clock_t end_time_test_item{0};

        start_time_all = clock();
        // foreach test set
        for (auto it = m_test_sets.begin(); it != m_test_sets.end(); it++) {
            std::string test_set_name = it->first;
            TestSet &test_set = it->second;

            int count = 0;
            for (size_t i = 0; i < test_set.size(); i++) {
                if (test_set[i].match_filter) count++;
            }
            if (count == 0) continue;

            info_one() << make_proper_str(count, "test", false) << " from "
                       << test_set_name << '\n';

            start_time_test_set = clock();
            // foreach test item
            for (size_t i = 0; i < test_set.size(); i++) {
                if (!test_set[i].match_filter) continue;

                const char *fullname = test_set[i].fullname;
                info_run() << fullname << '\n';

                int tmp_fail_count = 0;
                start_time_test_item = clock();
                test_set[i].f(&tmp_fail_count);  // call f
                end_time_test_item = clock();
                test_set[i].success = (tmp_fail_count == 0);

                if (test_set[i].success) {
                    info_ok() << fullname << " ("
                              << end_time_test_item - start_time_test_item
                              << " ms)\n";
                }
                else {
                    info_failed() << fullname << " ("
                                  << end_time_test_item - start_time_test_item
                                  << " ms)\n";

                    ++m_test_fail_count;
                }
            }  // foreach test item
            end_time_test_set = clock();

            info_one() << make_proper_str(count, "test", false) << " from "
                       << test_set_name << " ("
                       << end_time_test_set - start_time_test_set
                       << " ms total)\n\n";
        }  // foreach test set
        end_time_all = clock();
        m_cost_time_all = end_time_all - start_time_all;

        show_result();
        return m_test_fail_count;
    }

    void list_and_exit() {
        set_filter("*", false);
        set_matched_count();

        show_note_main();

        for (auto it = m_test_sets.begin(); it != m_test_sets.end(); it++) {
            msg_even_brief() << it->first << ".\n";

            const TestSet &test_set = it->second;
            for (size_t i = 0; i < test_set.size(); i++) {
                if (test_set[i].match_filter)
                    msg_even_brief() << "  " << test_set[i].itemname << '\n';
            }
        }

        exit(0);
    }

    void help_and_exit() const {
        show_note_main();
        show_logo();

        msg_even_brief() << "[USAGE]\n";
        msg_even_brief()
            << "This program contains tests written using MTest.\n"
            << "MTest is a gtest-style simple test framework by "
               "fenglielie@qq.com.\n"
            << "You can use the following command line flags to control "
               "its behavior:\n\n";
        msg_even_brief()
            << "1. --mtest_filter=POSITIVE_PATTERN\n"
            << "   Run only the tests whose name matches the positive "
               "pattern.\n"
            << "   '?' matches any single character; '*' matches any "
               "substring(or empty).\n";
        msg_even_brief()
            << "2. --mtest_list_tests\n"
            << "   List the names of all tests without running them.\n"
            << "The name of TEST(Foo, Bar) is \"Foo.Bar\".\n";
        msg_even_brief() << "3. --mtest_disable_color\n"
                         << "   Disable colorful output.\n";
        msg_even_brief() << "4. --mtest_brief\n"
                         << "   Enable brief output.\n";

        msg_even_brief() << '\n';
        exit(0);
    }

    void show_note_main() const {
        if (!m_main_file.empty() && m_main_file != "main.cpp") {
            msg_even_brief()
                << "Note: Running main() from " << m_main_file << '\n';
        }
    }

    void show_note_filter() const {
        if (m_filter != "*")
            msg_even_brief() << "Note: MTest filter = " << m_filter << '\n';
    }

    void show_start() const {
        show_logo();

        info_two() << "Running "
                   << make_proper_str(m_matched_count_items, "test", false)
                   << " from "
                   << make_proper_str(m_matched_count_sets, "test suite", false)
                   << ".\n";
        info_one() << "Global test environment set-up.\n";
    }

    void show_result() const {
        info_one() << "Global test environment tear-down\n";
        info_two() << make_proper_str(m_matched_count_items, "test", false)
                   << " from "
                   << make_proper_str(m_matched_count_sets, "test suite", false)
                   << " ran. (" << m_cost_time_all << " ms total)\n";

        info_passed() << make_proper_str(
            m_matched_count_items - m_test_fail_count, "test", false)
                      << ".\n";

        // list all failed tests
        if (m_test_fail_count != 0 && !MTestMessage::brief_output()) {
            info_failed() << make_proper_str(m_test_fail_count, "test", false)
                          << ", listed below:\n";

            for (auto it = m_test_sets.begin(); it != m_test_sets.end(); it++) {
                const TestSet &test_set = it->second;
                for (size_t i = 0; i < test_set.size(); i++) {
                    if (!test_set[i].success) {
                        info_failed() << test_set[i].fullname << '\n';
                    }
                }
            }
            if (m_use_color) {
                msg_even_brief()
                    << "\n " << m_color_red
                    << make_proper_str(m_test_fail_count, "FAILED TEST", true)
                    << m_color_end << '\n';
            }
            else {
                msg_even_brief()
                    << "\n "
                    << make_proper_str(m_test_fail_count, "FAILED TEST", true)
                    << '\n';
            }
        }
    }

    void show_logo() const {
        if (m_use_color) {
            msg() << m_color_green << "\
      __  __ _____ _____ ____ _____ \n\
     |  \\/  |_   _| ____/ ___|_   _|\n\
     | |\\/| | | | |  _| \\___ \\ | |\n\
     | |  | | | | | |___ ___) || |\n\
     |_|  |_| |_| |_____|____/ |_|\n"
                  << m_color_end << '\n';
        }
        else {
            msg() << "\
      __  __ _____ _____ ____ _____ \n\
     |  \\/  |_   _| ____/ ___|_   _|\n\
     | |\\/| | | | |  _| \\___ \\ | |\n\
     | |  | | | | | |___ ___) || |\n\
     |_|  |_| |_| |_____|____/ |_|\n"
                  << '\n';
        }
    }

    MTestMessage &info_passed() const {
        if (m_use_color) {
            msg_even_brief() << m_color_green << "[  PASSED  ] " << m_color_end;
        }
        else { msg_even_brief() << "[  PASSED  ] "; }
        return msg_even_brief();
    }

    MTestMessage &info_failed() const {
        if (m_use_color) {
            msg_even_brief() << m_color_red << "[  FAILED  ] " << m_color_end;
        }
        else { msg_even_brief() << "[  FAILED  ] "; }
        return msg_even_brief();
    }

    MTestMessage &info_ok() const {
        if (m_use_color) {
            msg() << m_color_green << "[       OK ] " << m_color_end;
        }
        else { msg() << "[       OK ] "; }
        return msg();
    }

    MTestMessage &info_two() const {
        if (m_use_color) {
            msg_even_brief() << m_color_green << "[==========] " << m_color_end;
        }
        else { msg_even_brief() << "[==========] "; }
        return msg_even_brief();
    }

    MTestMessage &info_one() const {
        if (m_use_color) {
            msg() << m_color_green << "[----------] " << m_color_end;
        }
        else { msg() << "[----------] "; }
        return msg();
    }

    MTestMessage &info_run() const {
        if (m_use_color) {
            msg() << m_color_green << "[ RUN      ] " << m_color_end;
        }
        else { msg() << "[ RUN      ] "; }
        return msg();
    }

    void set_test_item(const char *test_set_name, const char *test_itemname,
                       FuncType *f, const char *test_fullname) noexcept {
        try {
            TestItem item(f, test_itemname, test_fullname);
            m_test_sets[test_set_name].emplace_back(item);
        }
        catch (...) {
            std::cerr << "Failed to add test item: " << test_fullname << '\n';
            exit(1);
        }
    }

    void set_filter_force(std::string filter) { m_filter = filter; }

    void set_main_file(const char *main_file) { m_main_file = main_file; }

    void set_from_argv(int argc, char *argv[]) {
        if (argc == 1) return;

        const std::string filter_prefix = "--mtest_filter=";
        const std::string list_tests_option = "--mtest_list_tests";
        const std::string disable_color_option = "--mtest_disable_color";
        const std::string help_option = "--help";
        const std::string brief_option = "--mtest_brief";

        std::string arg_str;
        bool is_filter_override = false;

        for (int i = 1; i < argc; i++) {
            arg_str = argv[i];

            if (starts_with(arg_str, filter_prefix)) {  // filter
                if (!m_filter.empty()) is_filter_override = true;
                arg_str.erase(0, filter_prefix.length());

                set_filter(arg_str, true);  // force update
                set_matched_count();
            }
            else if (arg_str == disable_color_option)
                m_use_color = false;  // default: false
            else if (arg_str == brief_option)
                MTestMessage::brief_output() = true;  // default: false
            else if (arg_str == list_tests_option)
                list_and_exit();
            else if (arg_str == help_option)
                help_and_exit();
            else {
                msg_even_brief() << "Note: Unknown flag(" << arg_str
                                 << ") will be ignored.\n";
            }
        }

        if (is_filter_override)
            msg_even_brief()
                << "Note: MTest filter will be override by the last one.";
    }

    // update matched_count_items and matched_count_sets.
    // update test_item's match_filter.
    void set_matched_count() {
        m_matched_count_items = 0;
        m_matched_count_sets = 0;

        for (auto it = m_test_sets.begin(); it != m_test_sets.end(); it++) {
            TestSet &test_set = it->second;

            int count = 0;
            for (size_t i = 0; i < test_set.size(); i++) {
                std::string fullname = test_set[i].fullname;
                if (str_match(fullname, m_filter)) {
                    ++count;
                    test_set[i].match_filter = true;
                    ++m_matched_count_items;
                }
                else
                    test_set[i].match_filter = false;
            }

            if (count > 0) ++m_matched_count_sets;
        }
    }

    static bool starts_with(const std::string &str, const std::string &prefix) {
        return prefix.size() <= str.size()
               && std::equal(prefix.cbegin(), prefix.cend(), str.cbegin());
    }

    static std::string make_proper_str(int num, const std::string &str,
                                       bool uppercase) {
        std::string res;
        if (num > 1) {
            if (uppercase)
                res = std::to_string(num) + " " + str + "S";
            else
                res = std::to_string(num) + " " + str + "s";
        }
        else { res = std::to_string(num) + " " + str; }
        return res;
    }

    // string match (DP):
    // '?' matches any single character; '*' matches any substring(or empty).
    static bool str_match(std::string str, std::string pattern) {
        if (pattern == "*") return true;

        const size_t m = str.size();
        const size_t n = pattern.size();
        std::vector<bool> prev(m + 1, false);
        std::vector<bool> curr(m + 1, false);

        prev[0] = true;

        for (size_t i = 1; i <= n; i++) {
            bool flag = true;
            for (size_t ii = 0; ii < i; ii++) {
                if (pattern[ii] != '*') {
                    flag = false;
                    break;
                }
            }
            curr[0] = flag;

            for (size_t j = 1; j <= m; j++) {
                if (pattern[i - 1] == '*') {
                    curr[j] = (curr[j - 1] || prev[j]);
                }
                else if ((pattern[i - 1] == '?')
                         || (str[j - 1] == pattern[i - 1])) {
                    curr[j] = prev[j - 1];
                }
                else { curr[j] = false; }
            }
            prev = curr;
        }

        return prev[m];
    }

    // private data
    //----------------------------------------------------------------------------//

    const char *m_color_red = "\x1b[91m";
    const char *m_color_green = "\x1b[92m";
    const char *m_color_end = "\x1b[0m";

    std::map<const char *, TestSet> m_test_sets;
    int m_test_fail_count{0};  // Number of failed test items.
    clock_t m_cost_time_all{0};

    std::string m_filter;
    std::string m_main_file;  // The file name of main().
    bool m_use_color{true};   // Use colored output.

    int m_matched_count_items{0};  // Number of test items matched the filter.
    int m_matched_count_sets{0};   // Number of test sets matched the filter.
};

#ifndef UNUSE_MTEST

// EXPECT
#define MTEST_EXPECT(x, y, cond)                                               \
    if (!((x)cond(y))) {                                                       \
        MTest::MTestMessage::expect_result() = false;                          \
        MTest::msg_even_brief()                                                \
            << __FILE__ << ":" << __LINE__ << ": Failure\n";                   \
        if (strcmp(#cond, "==") == 0) {                                        \
            MTest::msg_even_brief() << "Expected equality of these values:\n"  \
                                    << "  " << #x << '\n';                     \
            MTest::MTestMessage::evaluate_if_required(#x, x);                  \
            MTest::msg_even_brief() << "  " << #y << '\n';                     \
            MTest::MTestMessage::evaluate_if_required(#y, y);                  \
        }                                                                      \
        else {                                                                 \
            MTest::msg_even_brief()                                            \
                << "Expected: (" << #x << ") " << #cond << " (" << #y          \
                << "), actual: " << std::to_string(x) << " vs "                \
                << std::to_string(y) << '\n';                                  \
        }                                                                      \
        *tmp_fail_count = *tmp_fail_count + 1;                                 \
    }                                                                          \
    else { MTest::MTestMessage::expect_result() = true; }                      \
    MTest::msg_when_fail_even_brief()

// EXPECT_XX
#define EXPECT_EQ(x, y) MTEST_EXPECT(x, y, ==)
#define EXPECT_NE(x, y) MTEST_EXPECT(x, y, !=)
#define EXPECT_LT(x, y) MTEST_EXPECT(x, y, <)
#define EXPECT_LE(x, y) MTEST_EXPECT(x, y, <=)
#define EXPECT_GT(x, y) MTEST_EXPECT(x, y, >)
#define EXPECT_GE(x, y) MTEST_EXPECT(x, y, >=)

// EXPECT double almost equal
#define EXPECT_NEAR(x, y, precision)                                           \
    if (std::abs((x) - (y)) > (precision)) {                                   \
        MTest::MTestMessage::expect_result() = false;                          \
        MTest::msg_even_brief()                                                \
            << __FILE__ << ":" << __LINE__ << ": Failure\n";                   \
        MTest::msg_even_brief()                                                \
            << "Expected: (" << #x << ") "                                     \
            << " ~ "                                                           \
            << " (" << #y << "), actual: " << std::to_string(x) << " vs "      \
            << std::to_string(y) << "(" << (precision) << ")\n";                 \
        *tmp_fail_count = *tmp_fail_count + 1;                                 \
    }                                                                          \
    else { MTest::MTestMessage::expect_result() = true; }                      \
    MTest::msg_when_fail_even_brief()

// EXPECT bool
#define EXPECT_TRUE(x)                                                         \
    if (!((x))) {                                                              \
        MTest::MTestMessage::expect_result() = false;                          \
        MTest::msg_even_brief()                                                \
            << __FILE__ << ":" << __LINE__ << ": Failure\n"                    \
            << "Value of: " << #x << '\n'                                      \
            << "  Actual: false\n"                                             \
            << "Expected: true\n";                                             \
        *tmp_fail_count = *tmp_fail_count + 1;                                 \
    }                                                                          \
    else { MTest::MTestMessage::expect_result() = true; }                      \
    MTest::msg_when_fail_even_brief()

#define EXPECT_FALSE(x)                                                        \
    if (((x))) {                                                               \
        MTest::MTestMessage::expect_result() = false;                          \
        MTest::msg_even_brief()                                                \
            << __FILE__ << ":" << __LINE__ << ": Failure\n"                    \
            << "Value of: " << #x << '\n'                                      \
            << "  Actual: true\n"                                              \
            << "Expected: false\n";                                            \
        *tmp_fail_count = *tmp_fail_count + 1;                                 \
    }                                                                          \
    else { MTest::MTestMessage::expect_result() = true; }                      \
    MTest::msg_when_fail_even_brief()

// ASSERT
#define MTEST_ASSERT(x, y, cond)                                               \
    if (!((x)cond(y))) {                                                       \
        MTest::msg_even_brief()                                                \
            << __FILE__ << ":" << __LINE__ << ": Failure\n";                   \
        if (strcmp(#cond, "==") == 0) {                                        \
            MTest::msg_even_brief() << "Expected equality of these values:\n"  \
                                    << "  " << #x << '\n';                     \
            MTest::MTestMessage::evaluate_if_required(#x, x);                  \
            MTest::msg_even_brief() << "  " << #y << '\n';                     \
            MTest::MTestMessage::evaluate_if_required(#y, y);                  \
        }                                                                      \
        else {                                                                 \
            MTest::msg_even_brief()                                            \
                << "Expected: (" << #x << ") " << #cond << " (" << #y          \
                << "), actual: " << std::to_string(x) << " vs "                \
                << std::to_string(y) << '\n';                                  \
        }                                                                      \
        *tmp_fail_count = *tmp_fail_count + 1;                                 \
        return;                                                                \
    }

// ASSERT_XX
#define ASSERT_EQ(x, y) MTEST_ASSERT(x, y, ==)
#define ASSERT_NE(x, y) MTEST_ASSERT(x, y, !=)
#define ASSERT_LT(x, y) MTEST_ASSERT(x, y, <)
#define ASSERT_LE(x, y) MTEST_ASSERT(x, y, <=)
#define ASSERT_GT(x, y) MTEST_ASSERT(x, y, >)
#define ASSERT_GE(x, y) MTEST_ASSERT(x, y, >=)

// ASSERT bool
#define ASSERT_TRUE(x)                                                         \
    if (!((x))) {                                                              \
        MTest::msg_even_brief()                                                \
            << __FILE__ << ":" << __LINE__ << ": Failure\n"                    \
            << "Value of: " << #x << '\n'                                      \
            << "  Actual: false\n"                                             \
            << "Expected: true\n";                                             \
        *tmp_fail_count = *tmp_fail_count + 1;                                 \
        return;                                                                \
    }

#define ASSERT_FALSE(x)                                                        \
    if (((x))) {                                                               \
        MTest::msg_even_brief()                                                \
            << __FILE__ << ":" << __LINE__ << ": Failure\n"                    \
            << "Value of: " << #x << '\n'                                      \
            << "  Actual: true\n"                                              \
            << "Expected: false\n";                                            \
        *tmp_fail_count = *tmp_fail_count + 1;                                 \
        return;                                                                \
    }

// TEST
#define TEST(set, name)                                                        \
    void mtest_donotuse_func_##set##_##name(int *tmp_fail_count);              \
    struct MtestMarkClass##set##name {                                         \
    private:                                                                   \
        const static inline int m_mtest_donotuse_mark_##set##_##name =         \
            MTest::add_test_item(#set, #name,                                  \
                                 mtest_donotuse_func_##set##_##name,           \
                                 #set "." #name);                              \
    };                                                                         \
    void mtest_donotuse_func_##set##_##name(int *tmp_fail_count)

#define RUN_ALL_TESTS MTest::run_all_tests

#define TEST_FILTER(filter_str) MTest::set_filter(filter_str)

#define MTEST_MAIN                                                             \
    int main(int argc, char *argv[]) {                                         \
        MTest::init_mtest(argc, argv, __FILE__);                               \
        return MTest::run_all_tests();                                         \
    }

#else
#define MTEST_MAIN
#endif  // UNUSE_MTEST

#endif  // MTEST_H_
