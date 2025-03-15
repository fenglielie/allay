#include "allay/mtest/mtest.hpp"
#include "allay/mtest/mtest_macro.hpp"

namespace {

TEST(Equal, 1) { EXPECT_EQ(1, 1) << "is 1==1 ?"; }

TEST(Equal, 2) { EXPECT_NE(2, 3) << "is 2!=3 ?"; }

TEST(NotEqual, 1) { EXPECT_NE(2, 3) << "is 2!=3 ?"; }

TEST(IsTrue, 1) { EXPECT_TRUE(2 < 3); }

TEST(isTrue, 2) { EXPECT_FALSE(2 > 4); }

}  // namespace

MTEST_MAIN
