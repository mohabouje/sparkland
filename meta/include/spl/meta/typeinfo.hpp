#pragma once

#include "spl/meta/tstring.hpp"

#include <cctype>
#include <cstdint>
#include <string>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <cxxabi.h>
#include <cstdlib>
#include <array>
#include <utility>

namespace spl { inline namespace meta {

    namespace internal {
        template <std::size_t... Idxs>
        constexpr auto substring_as_array(std::string_view str, std::index_sequence<Idxs...>) {
            return std::array{str[Idxs]..., '\0'};
        }

        template <typename T>
        constexpr auto type_name_array() {
#if defined(__clang__)
            constexpr auto prefix   = std::string_view{"[T = "};
            constexpr auto suffix   = std::string_view{"]"};
            constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(__GNUC__)
            constexpr auto prefix   = std::string_view{"with T = "};
            constexpr auto suffix   = std::string_view{"]"};
            constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(_MSC_VER)
            constexpr auto prefix   = std::string_view{"type_name_array<"};
            constexpr auto suffix   = std::string_view{">(void)"};
            constexpr auto function = std::string_view{__FUNCSIG__};
#else
#    error Unsupported compiler
#endif

            constexpr auto start = function.find(prefix) + prefix.size();
            constexpr auto end   = function.rfind(suffix);

            static_assert(start < end);

            constexpr auto name = function.substr(start, (end - start));
            return substring_as_array(name, std::make_index_sequence<name.size()>{});
        }

        template <typename T>
        struct type_name_holder {
            inline constexpr static auto value = type_name_array<T>();
        };
    } // namespace internal

    template <typename T>
    constexpr auto type() -> std::string_view {
        constexpr auto& value = internal::type_name_holder<T>::value;
        return std::string_view{value.data(), value.size() - sizeof('\n')};
    }

    template <typename T>
    constexpr auto type_name() -> std::string_view {
        constexpr auto value = type<T>();
        constexpr auto first = [&]() -> std::size_t {
            for (std::size_t i = 0; i < std::size(value); ++i) {
                if (value[i] == '<') {
                    return i;
                }
            }
            return std::size(value) - 1;
        }();
        for (std::int32_t i = first; i >= 0; --i) {
            if (value[i] == ':') {
                return value.substr(i + 1, std::size(value) - i - 1);
            }
        }
        return value;
    }

    template <typename T>
    constexpr auto type_namespace() -> std::string_view {
        constexpr auto value = type<T>();
        for (std::int32_t i = std::size(value) - 1; i >= 0; --i) {
            if (value[i] == ':') {
                return value.substr(0, i - 1);
            }
        }
        return value;
    }

    static_assert(type_name<int>() == "int");
    static_assert(type<std::pair<int, int>>() == "std::pair<int, int>");
    static_assert(type_name<std::pair<int, int>>() == "pair<int, int>");
    static_assert(type_namespace<std::pair<int, int>>() == "std");

}} // namespace spl::meta
