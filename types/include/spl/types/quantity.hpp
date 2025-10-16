#pragma once

#include "spl/types/decimal.hpp"

namespace spl::types {

    using quantity = decimal<10, std::int64_t>;

    static_assert(std::is_trivially_destructible_v<quantity>,
                  "spl::types::quantity must be a trivially_destructible object");

} // namespace spl::types

[[nodiscard]] constexpr auto operator""_q(long double value) noexcept -> spl::types::quantity {
    return spl::types::quantity::from(value);
}

[[nodiscard]] constexpr auto operator""_q(char const* value) noexcept -> spl::types::quantity {
    return spl::types::quantity::from(value);
}