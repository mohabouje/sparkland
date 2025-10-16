#pragma once

#include "spl/concepts/types.hpp"
#include "spl/types/decimal.hpp"

#include <cmath>
#include <cstdint>
#include <type_traits>

namespace spl::math {

    inline constexpr auto EPSILON = 0.0000001;
    inline constexpr auto epsilon = 0.0000001;

    template <spl::concepts::strictly_integral IntegerType>
    [[gnu::always_inline]] constexpr auto log2(IntegerType n, IntegerType p = 0) noexcept -> IntegerType {
        return (n <= 1) ? p : log2(n / 2, p + 1);
    }

    template <spl::concepts::strictly_integral IntegerType>
    [[gnu::always_inline]] constexpr auto closest_exp_2(IntegerType size) noexcept -> IntegerType {
        return size == 1 ? 1 : (1UL << (static_cast<std::uint64_t>(log2(size - 1)) + 1));
    }

    template <spl::concepts::strictly_integral IntegerType>
    [[gnu::always_inline]] constexpr auto round(double value) noexcept -> IntegerType {
        return static_cast<IntegerType>(value < 0 ? (value - 0.5) : (value + 0.5));
    }

    template <spl::concepts::unsigned_integral UnsignedT>
    [[gnu::always_inline]] constexpr auto abs(UnsignedT x) noexcept -> UnsignedT {
        return x;
    }

    template <spl::concepts::signed_integral SignedT>
    [[gnu::always_inline]] constexpr auto abs(SignedT x) noexcept -> SignedT {
        return (x >= 0) ? x : -x;
    }

    template <spl::concepts::floating_point FloatT>
    [[gnu::always_inline]] constexpr auto approximately_eq(FloatT a, FloatT b) noexcept -> bool {
        return std::fabs(a - b) < EPSILON;
    }

    template <spl::concepts::floating_point FloatT>
    [[gnu::always_inline]] constexpr auto essentially_eq(FloatT a, FloatT b) noexcept -> bool {
        return std::fabs(a - b) < EPSILON;
    }

    template <spl::concepts::floating_point FloatT>
    [[gnu::always_inline]] constexpr auto definitely_gt(FloatT a, FloatT b) noexcept -> bool {
        return a - b >= EPSILON;
    }

    template <spl::concepts::floating_point FloatT>
    [[gnu::always_inline]] constexpr auto definitely_lt(FloatT a, FloatT b) noexcept -> bool {
        return a - b <= -EPSILON;
    }

    template <spl::concepts::floating_point FloatT>
    [[gnu::always_inline]] constexpr auto definitely_get(FloatT a, FloatT b) noexcept -> bool {
        return approximately_eq(a, b) or definitely_gt(a, b);
    }

    template <spl::concepts::floating_point FloatT>
    [[gnu::always_inline]] constexpr auto definitely_let(FloatT a, FloatT b) noexcept -> bool {
        return approximately_eq(a, b) or definitely_lt(a, b);
    }

    template <spl::concepts::floating_point FloatT>
    [[gnu::always_inline]] constexpr auto positive(FloatT a) noexcept -> bool {
        return definitely_gt(a, FloatT{0}) and a > math::EPSILON;
    }

    template <spl::concepts::floating_point FloatT>
    [[gnu::always_inline]] constexpr auto negative(FloatT a) noexcept -> bool {
        return definitely_lt(a, FloatT{0}) and a < -math::EPSILON;
    }

    template <spl::concepts::floating_point FloatT>
    [[gnu::always_inline]] constexpr auto zero(FloatT a) noexcept -> bool {
        return not positive(a) and not negative(a);
    }

    template <spl::concepts::floating_point FloatT>
    [[gnu::always_inline]] constexpr auto finite(FloatT a) noexcept -> bool {
        return std::isfinite(a);
    }

    template <std::int16_t PrecisionV, concepts::strictly_integral MantissaT>
    [[gnu::always_inline]] constexpr auto approximately_eq(spl::types::decimal<PrecisionV, MantissaT> a,
                                                           spl::types::decimal<PrecisionV, MantissaT> b) noexcept
        -> bool {
        return a == b;
    }

    template <std::int16_t PrecisionV, concepts::strictly_integral MantissaT>
    [[gnu::always_inline]] constexpr auto essentially_eq(spl::types::decimal<PrecisionV, MantissaT> a,
                                                         spl::types::decimal<PrecisionV, MantissaT> b) noexcept
        -> bool {
        return a == b;
    }

    template <std::int16_t PrecisionV, concepts::strictly_integral MantissaT>
    [[gnu::always_inline]] constexpr auto definitely_gt(spl::types::decimal<PrecisionV, MantissaT> a,
                                                        spl::types::decimal<PrecisionV, MantissaT> b) noexcept -> bool {
        return a > b;
    }

    template <std::int16_t PrecisionV, concepts::strictly_integral MantissaT>
    [[gnu::always_inline]] constexpr auto definitely_lt(spl::types::decimal<PrecisionV, MantissaT> a,
                                                        spl::types::decimal<PrecisionV, MantissaT> b) noexcept -> bool {
        return a < b;
    }

    template <std::int16_t PrecisionV, concepts::strictly_integral MantissaT>
    [[gnu::always_inline]] constexpr auto definitely_get(spl::types::decimal<PrecisionV, MantissaT> a,
                                                         spl::types::decimal<PrecisionV, MantissaT> b) noexcept
        -> bool {
        return a >= b;
    }

    template <std::int16_t PrecisionV, concepts::strictly_integral MantissaT>
    [[gnu::always_inline]] constexpr auto definitely_let(spl::types::decimal<PrecisionV, MantissaT> a,
                                                         spl::types::decimal<PrecisionV, MantissaT> b) noexcept
        -> bool {
        return a <= b;
    }

    template <std::int16_t PrecisionV, concepts::strictly_integral MantissaT>
    [[gnu::always_inline]] constexpr auto positive(spl::types::decimal<PrecisionV, MantissaT> a) noexcept -> bool {
        return a.is_positive();
    }

    template <std::int16_t PrecisionV, concepts::strictly_integral MantissaT>
    [[gnu::always_inline]] constexpr auto negative(spl::types::decimal<PrecisionV, MantissaT> a) noexcept -> bool {
        return a.is_negative();
    }

    template <std::int16_t PrecisionV, concepts::strictly_integral MantissaT>
    [[gnu::always_inline]] constexpr auto zero(spl::types::decimal<PrecisionV, MantissaT> a) noexcept -> bool {
        return a.is_zero();
    }

    template <std::int16_t PrecisionV, concepts::strictly_integral MantissaT>
    [[gnu::always_inline]] constexpr auto finite(spl::types::decimal<PrecisionV, MantissaT> a) noexcept -> bool {
        return a.is_finite();
    }

    template <std::int16_t PrecisionV, concepts::strictly_integral MantissaT>
    [[gnu::always_inline]] constexpr auto invalid(spl::types::decimal<PrecisionV, MantissaT> a) noexcept -> bool {
        return not a.is_finite();
    }

} // namespace spl::math
