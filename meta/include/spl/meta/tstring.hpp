#pragma once

#include <cstdint>
#include <string_view>
#include <tuple>
#include <type_traits>

namespace spl { inline namespace meta {

    template <std::size_t N>
    struct tstring {
        static_assert(N > 0);
        char const data[N]{}; // NOLINT

    private:
        template <std::size_t... Is>
        consteval tstring(char const (&ptr)[N], std::index_sequence<Is...>) : data{ptr[Is]...} {} // NOLINT

        template <std::size_t... Is>
        consteval tstring(std::array<char, N> const& value, std::index_sequence<Is...>) : data{value[Is]...} {}

    public:
        consteval tstring(char const (&ptr)[N]) : tstring(ptr, std::make_index_sequence<N>{}) {} // NOLINT

        consteval tstring(std::array<char, N> const& value) : tstring(value, std::make_index_sequence<N>{}) {} // NOLINT

        template <char... Char>
        consteval tstring() : data{Char...} {}

        [[nodiscard]] constexpr auto begin() const noexcept -> char const* {
            return data;
        }

        [[nodiscard]] constexpr auto end() const noexcept -> char const* {
            return data + static_cast<std::ptrdiff_t>(size());
        }

        [[nodiscard]] constexpr static auto size() noexcept -> std::size_t {
            return N - 1;
        }

        template <std::size_t M>
        [[nodiscard]] constexpr auto operator==(tstring<M> const& rhs) const noexcept -> bool {
            if (N != M) {
                return false;
            }
            for (auto index = 0ZU; index < N; ++index) {
                if (data[index] != rhs.data[index]) {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] constexpr auto operator==(std::string_view rhs) const noexcept -> bool {
            return std::string_view(data, N - 1) == rhs;
        }

        [[nodiscard]] explicit constexpr operator std::string_view() const noexcept {
            return std::string_view(data, N - 1);
        }

        [[nodiscard]] explicit constexpr operator char const*() const noexcept {
            return data;
        }

        [[nodiscard]] constexpr auto view() const noexcept -> std::string_view {
            return std::string_view(data, N - 1);
        }
    };

    template <typename... Chars>
    tstring(Chars...) -> tstring<sizeof...(Chars)>;

    template <std::size_t N>
    tstring(char const (&)[N]) -> tstring<N>; // NOLINT

}} // namespace spl::meta
