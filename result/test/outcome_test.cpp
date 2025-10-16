#include "spl/result/result.hpp"

#include <gtest/gtest.h>
#include <variant>

TEST(BoostOutcomeTest, MakeFormattedError) {
    auto error = spl::outcome::failure("This is an example of an error {}", 19.0);
    EXPECT_EQ(error.error().what(), "This is an example of an error 19");
}

TEST(BoolTest, BoolConversion) {
    [[maybe_unused]] auto const expression = [&]() -> spl::result<int> {
        auto const operation = []() { return spl::result<int>(9); };
        auto const result    = err_return(operation());
        EXPECT_EQ(result, 9);
        return result;
    }();
}
