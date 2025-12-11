#include "allay/mtest/mtest.hpp"

namespace {

TEST(Equal, e1) { EXPECT_EQ(1, 1) << "is 1==1 ?"; }

TEST(Equal, e2) { EXPECT_NE(2, 3) << "is 2!=3 ?"; }

TEST(NotEqual, ne) { EXPECT_NE(2, 3) << "is 2!=3 ?"; }

TEST(IsTrue, it1) { EXPECT_TRUE(2 < 3); }

TEST(IsTrue, it2) { EXPECT_FALSE(2 > 4); }

TEST(IsTrue, it3) { EXPECT_NEAR(2, 2.05, 0.1); }

TEST(NotEqual, DISABLED_ne2) { EXPECT_NE(2, 3) << "is 2!=3 ?"; }


}  // namespace

MTEST_MAIN
