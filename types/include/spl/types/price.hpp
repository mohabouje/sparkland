#pragma once

#include "spl/types/decimal.hpp"

namespace spl::types {

    using price = decimal<8, std::int64_t>;

    static_assert(std::is_trivially_destructible_v<price>, "spl::types::price must be a trivially_destructible object");

} // namespace spl::types

[[nodiscard]] constexpr auto operator""_p(long double value) noexcept -> spl::types::price {
    return spl::types::price::from(value);
}

[[nodiscard]] constexpr auto operator""_p(char const* value) noexcept -> spl::types::price {
    return spl::types::price::from(value);
}