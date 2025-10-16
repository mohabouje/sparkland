#pragma once

#include <type_traits>
#include <utility>

namespace spl::inline meta {

    template <typename T>
    [[nodiscard]] constexpr auto vmin(T&& t) noexcept -> T {
        return std::forward<T>(t);
    }

    template <typename T0, typename T1, typename... Ts>
    [[nodiscard]] constexpr auto vmin(T0&& val1, T1&& val2, Ts&&... vs) noexcept ->
        typename std::common_type<T0, T1, Ts...>::type {
        if (val2 < val1) {
            return vmin(val2, std::forward<Ts>(vs)...);
        } else {
            return vmin(val1, std::forward<Ts>(vs)...);
        }
    }

} // namespace spl::inline meta