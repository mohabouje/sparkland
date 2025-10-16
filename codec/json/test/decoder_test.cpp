#include "spl/codec/json/json.hpp"
#include "spl/reflect/enum.hpp"
#include "spl/reflect/object.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

struct simple {
    int number{};
    float floating{};
    std::optional<int> optional{};
};

struct nested_struct {
    int number{};
    ::simple simple{};
};

enum class example { one, two, three };

using namespace spl;

TEST(CodecJsonDecoding, DecodeSimpleObject) {
    std::string_view const raw = R"({"number": 42, "floating": 10.5, "numbers": [1, 2, 3], "optional": 42})";

    auto const decoded = spl::codec::json::from_json<simple>(raw);
    ASSERT_TRUE(decoded) << decoded.error().message().data();
    EXPECT_EQ(decoded.value().number, 42);
    ASSERT_TRUE(decoded.value().optional);
    EXPECT_EQ(*decoded.value().optional, 42);
}

TEST(CodecJsonDecoding, DecodeSimpleObjectMissingOptionalField) {
    std::string_view const raw = R"({"number": 42, "floating": 10.5, "text": "hello", "numbers": [1, 2, 3]})";

    auto const decoded = spl::codec::json::from_json<simple>(raw);
    ASSERT_TRUE(decoded) << decoded.error().message().data();
    EXPECT_EQ(decoded.value().number, 42);
    ASSERT_FALSE(decoded.value().optional);
}

TEST(CodecJsonDecoding, DecodeSimpleObjectMissingRequiredField) {
    std::string_view const raw = R"({"number": 42, "floating": 10.5, "text": "hello"})";

    auto const decoded = spl::codec::json::from_json<simple>(raw);
    ASSERT_TRUE(decoded) << decoded.error().message().data();
    ASSERT_FALSE(decoded.value().optional);
}

TEST(CodecJsonDecoding, DecodeNestedStructObject) {
    std::string_view const raw =
        R"({"number" : 10, "simple" :  {"number": 42, "floating": 10.5, "text": "hello", "numbers": [1, 2, 3], "optional": 42}})";

    auto const decoded = spl::codec::json::from_json<nested_struct>(raw);
    ASSERT_TRUE(decoded) << decoded.error().message().data();
    EXPECT_EQ(decoded.value().number, 10);
    ASSERT_TRUE(decoded.value().simple.optional);
    EXPECT_EQ(*decoded.value().simple.optional, 42);
}

TEST(CodecJsonDecoding, DecodeNestedStructMissingOptionalField) {
    std::string_view const raw =
        R"({"number" : 10, "simple" :  {"number": 42, "floating": 10.5, "text": "hello", "numbers": [1, 2, 3]}})";

    auto const decoded = spl::codec::json::from_json<nested_struct>(raw);
    EXPECT_EQ(decoded.value().number, 10);
    ASSERT_FALSE(decoded.value().simple.optional);
}
