#pragma once

#include "spl/result/result.hpp"
#include "spl/codec/json/decoder.hpp"
#include "spl/codec/json/encoder.hpp"

namespace spl::codec::json {

    namespace internal {
        template <char... Tag>
        [[nodiscard]] constexpr auto find_tag(std::span<char const> input) noexcept -> std::span<char const> {
            constexpr auto length = sizeof...(Tag);
            auto const maximum    = std::size(input) - length;
            for (auto i = 0ULL; i < maximum; ++i) {
                auto index        = i;
                auto const found  = ((Tag == input[index++]) && ...);
                auto const closed = input[index] == '"';
                if (found & closed) {
                    return input.subspan(i);
                }
            }
            return {};
        }

        template <typename OutputType, char... Tag>
        [[nodiscard]] constexpr auto get_tag(std::span<char const> input) noexcept -> std::span<char const> {
            auto const result                 = find_tag<Tag...>(std::string_view{std::data(input), std::size(input)});
            constexpr auto string_type        = std::is_convertible_v<OutputType, std::string_view>;
            constexpr auto skipped_characters = sizeof...(Tag) + 2 + string_type;
            auto const maximum                = std::size(result);
            for (auto i = skipped_characters; i < maximum; ++i) {
                auto const character = result[i];
                if (character == ',' or character == '}') {
                    auto const value = result.subspan(skipped_characters, i - skipped_characters - string_type);
                    return value;
                }
            }
            return {};
        }

        template <typename TagType, char... Tag, typename ValueT>
        [[nodiscard]] constexpr auto set_tag(std::span<char> input, ValueT value) noexcept -> std::span<char> {
            constexpr auto length           = sizeof...(Tag);
            constexpr auto string_type      = std::is_convertible_v<std::decay_t<TagType>, std::string_view>;
            constexpr auto minimum_required = length + 2 * string_type + 4;
            auto const original_size        = std::size(input);
            auto const dummy_struct         = static_cast<std::size_t>(original_size <= 2);
            auto const appended_size        = std::size(input) + std::size(value) + minimum_required - dummy_struct;
            input                           = std::span<char>{std::data(input), appended_size};

            auto position   = original_size - 1;
            input[position] = ',';
            position        = position + static_cast<std::size_t>(not dummy_struct);
            input[position] = '"';

            ((input[++position] = Tag), ...);

            input[++position] = '"';
            input[++position] = ':';

            if constexpr (string_type) {
                input[++position] = '"';
            }

            for (auto i = 0ULL; i < std::size(value); ++i) {
                input[++position] = value[i];
            }

            if constexpr (string_type) {
                input[++position] = '"';
            }

            input[++position] = '}';
            return input;
        }

    } // namespace internal

    struct tagger {
        template <char... Tag>
        [[nodiscard]] static auto set_tag(std::string_view value, std::span<char> input) -> std::span<char> {
            return internal::set_tag<std::string_view, Tag...>(input, value);
        }

        template <typename OtherT, char... Tag>
        [[nodiscard]] static auto set_tag(std::string_view value, std::span<char> input) -> std::span<char> {
            return internal::set_tag<OtherT, Tag...>(input, value);
        }

        template <char... Tag>
        [[nodiscard]] static auto get_tag(std::span<char const> input) -> std::string_view {
            auto const temporal = internal::get_tag<std::string_view, Tag...>(input);
            return std::string_view{std::data(temporal), std::size(temporal)};
        }

        template <typename OutputT, char... Tag>
        [[nodiscard]] static auto get_tag(std::span<char const> input) -> std::string_view {
            auto const temporal = internal::get_tag<OutputT, Tag...>(input);
            return std::string_view{std::data(temporal), std::size(temporal)};
        }
    };

} // namespace spl::codec::json
