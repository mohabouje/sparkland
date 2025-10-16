#include "spl/result/result.hpp"

#include <gtest/gtest.h>
#include <memory>

TEST(ResultTypeTest, ForwardErrorMessagesByRValue) {
    auto const result = []() -> spl::result<int> {
        auto intermediate = []() -> spl::result<void> {
            return spl::failure("Here's an example of an error: {}", "example");
        }();
        if (spl::failed(intermediate)) {
            return intermediate.error();
        }
        return 0;
    }();
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().value().what(), "Here's an example of an error: example");
}

TEST(ResultTypeTest, ForwardErrorMessagesByLeftValue) {
    auto const result = []() -> spl::result<int, std::string> {
        auto const intermediate = []() -> spl::result<void, std::string> {
            return spl::failure<std::string>("Here's an example of an error");
        }();
        if (spl::failed(intermediate)) {
            return spl::propagate(intermediate);
        }
        return 0;
    }();
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), "Here's an example of an error");
}

TEST(ResultTypeTest, MakeFormatterErrorMessage) {
    auto const result = []() -> spl::result<void> {
        return spl::failure("Here's an example of an error: {}", "example");
    }();
    ASSERT_FALSE(result);
    EXPECT_EQ(std::string_view(result.error().message().data(), result.error().message().size()),
              "Here's an example of an error: example");
}

TEST(ResultTypeTest, AccessReferenceValue) {
    auto example      = int(0);
    auto const result = [&example]() -> spl::result_ref<int> { return example; }();

    ASSERT_TRUE(result);
    EXPECT_EQ(std::addressof(example), std::addressof(result.value().get()));
}

TEST(ResultTypeTest, AccessConstReferenceValue) {
    auto example      = int(0);
    auto const result = [&example]() -> spl::result_cref<int> { return example; }();

    ASSERT_TRUE(result);
    EXPECT_EQ(std::addressof(example), std::addressof(result.value().get()));
}
