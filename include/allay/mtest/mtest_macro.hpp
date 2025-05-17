#ifndef MTEST_MACRO_H_
#define MTEST_MACRO_H_

#ifndef UNUSE_MTEST

// EXCEPT
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

// EXCEPT_XX
#define EXPECT_EQ(x, y) MTEST_EXPECT(x, y, ==)
#define EXPECT_NE(x, y) MTEST_EXPECT(x, y, !=)
#define EXPECT_LT(x, y) MTEST_EXPECT(x, y, <)
#define EXPECT_LE(x, y) MTEST_EXPECT(x, y, <=)
#define EXPECT_GT(x, y) MTEST_EXPECT(x, y, >)
#define EXPECT_GE(x, y) MTEST_EXPECT(x, y, >=)

// EXPECT double almost equal
#define EXPECT_AE(x, y, precision)                                             \
    if (std::abs((x) - (y)) > (precision)) {                                   \
        MTest::MTestMessage::expect_result() = false;                          \
        MTest::msg_even_brief()                                                \
            << __FILE__ << ":" << __LINE__ << ": Failure\n";                   \
        MTest::msg_even_brief()                                                \
            << "Expected: (" << #x << ") "                                     \
            << " ~ "                                                           \
            << " (" << #y << "), actual: " << std::to_string(x) << " vs "      \
            << std::to_string(y) << "(" << precision << ")\n";                 \
        *tmp_fail_count = *tmp_fail_count + 1;                                 \
    }                                                                          \
    else { MTest::MTestMessage::expect_result() = true; }                      \
    MTest::msg_when_fail_even_brief()

// EXCEPT bool
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

#endif  // MTEST_MACRO_H_
