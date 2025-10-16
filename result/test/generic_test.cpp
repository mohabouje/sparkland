#include "spl/result/result.hpp"

#include <gtest/gtest.h>

TEST(GenericErrorTest, DifferentInstancesAreEqualIfSameErrorMessage) {
    auto const error1 = make_status_code(spl::outcome::error_message("Example {}", 19.0));
    auto const error2 = make_status_code(spl::outcome::error_message("Example {}", 19.0));
    EXPECT_EQ(error1, error2);
}

TEST(GenericErrorTest, DifferentInstancesAreNotEqualIfDifferentErrorMessage) {
    auto const error1 = make_status_code(spl::outcome::error_message("Example {}", 19.0));
    auto const error2 = make_status_code(spl::outcome::error_message("Example {}", 20.0));
    EXPECT_NE(error1, error2);
}

TEST(GenericErrorTest, HandleErrorCase) {
    {
        SCOPED_TRACE("Integer Value");
        auto m = spl::outcome::result<int>(spl::generic_error{spl::outcome::error_message("Example {}", 19.0)});
        ASSERT_FALSE(m);
        ASSERT_FALSE(m.has_value());
        ASSERT_TRUE(m.has_error());
        ASSERT_EXIT({ m.value(); }, ::testing::KilledBySignal(SIGABRT), ".*");
        EXPECT_EQ(m.error().value().what(), "Example 19");
    }

    {
        SCOPED_TRACE("No Value");
        auto m = spl::outcome::result<void>(spl::generic_error{spl::outcome::error_message("Example {}", 19.0)});
        ASSERT_FALSE(m);
        ASSERT_FALSE(m.has_value());
        ASSERT_TRUE(m.has_error());
        ASSERT_EXIT({ m.value(); }, ::testing::KilledBySignal(SIGABRT), ".*");
        EXPECT_EQ(m.error().value().what(), "Example 19");
    }
}

TEST(GenericErrorTest, HandleValidCase) {
    {
        SCOPED_TRACE("Integer Value");
        auto m = spl::outcome::result<int>(5);
        ASSERT_TRUE(m);
        ASSERT_TRUE(m.has_value());
        ASSERT_FALSE(m.has_error());
        ASSERT_EXIT({ m.error(); }, ::testing::KilledBySignal(SIGABRT), ".*");
        EXPECT_EQ(m.value(), 5);
    }

    {
        SCOPED_TRACE("No Value");
        auto m = spl::outcome::result<void>(boost::outcome_v2::success());
        ASSERT_TRUE(m);
        ASSERT_TRUE(m.has_value());
        ASSERT_FALSE(m.has_error());
        ASSERT_EXIT({ m.error(); }, ::testing::KilledBySignal(SIGABRT), ".*");
    }
}
