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

TEST(CodecJsonEncoder, InspectContractCompileTime) {
    auto const example       = simple{42, 17.5, {42}};
    auto const hrm_forwarded = spl::reflect::forward_as_tuple(example);
    auto const daw_forwarded = daw::json::json_data_contract<simple>::to_json_data(example);
    static_assert(std::is_same_v<decltype(hrm_forwarded), decltype(daw_forwarded)>);
};

TEST(CodecJsonEncoder, EncodingSimpleStruct) {
    auto const example = simple{42, 17.5, {42}};
    auto const encoded = spl::codec::json::to_json<std::string>(example);
    EXPECT_EQ(encoded.value(), R"({"number":42,"floating":17.5,"optional":42})");
}

TEST(CodecJsonEncoder, EncodingNestedStruct) {
    auto const example = nested_struct{
        42, {42, 17.5, {42}}
    };
    auto const encoded = spl::codec::json::to_json<std::string>(example);
    EXPECT_EQ(encoded.value(), R"({"number":42,"simple":{"number":42,"floating":17.5,"optional":42}})");
}

TEST(CodecJsonEncoder, EncodingNestedStructMissingOptionalArgument) {
    auto const example = nested_struct{
        42, {42, 17.5, {}}
    };
    auto const encoded = spl::codec::json::to_json<std::string>(example);
    EXPECT_EQ(encoded.value(), R"({"number":42,"simple":{"number":42,"floating":17.5}})");
}

struct simple_with_array {
    int number{};
    float floating{};
    std::optional<int> optional{};
    std::vector<simple> simples{};
};

TEST(CodecJsonEncoder, EncodeArrayOfElements) {
    auto const example = simple_with_array{
        .number   = 42,
        .floating = 17.5,
        .optional = 42,
        .simples  = {
                     {42, 17.5, {42}},
                     {42, 17.5, {42}},
                     {42, 17.5, {42}},
                     }
    };
    auto const encoded = spl::codec::json::to_json<std::string>(example);
    EXPECT_EQ(
        encoded.value(),
        R"({"number":42,"floating":17.5,"optional":42,"simples":[{"number":42,"floating":17.5,"optional":42},{"number":42,"floating":17.5,"optional":42},{"number":42,"floating":17.5,"optional":42}]})");
}