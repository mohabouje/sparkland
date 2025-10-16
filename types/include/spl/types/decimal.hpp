#pragma once

#include "spl/concepts/types.hpp"
#include "spl/core/assert.hpp"

#include <concepts>
#include <cstdint>
#include <limits>
#include <string>
#include <algorithm>

namespace spl::types {

    enum class decimal_format : bool { padded, trimmed };

    template <std::int16_t PrecisionV, concepts::strictly_integral MantissaT = std::int64_t>
    class decimal {
    public:
        using mantissa_type   = MantissaT;
        using precision_type  = decltype(PrecisionV);
        using exponent_type   = decltype(PrecisionV);
        using floating_type   = double;
        using reference       = decimal<PrecisionV, MantissaT>&;
        using const_reference = decimal<PrecisionV, MantissaT> const&;
        using string_type     = std::string;

        constexpr decimal() noexcept                                  = default;
        constexpr decimal(decimal const&) noexcept                    = default;
        constexpr decimal(decimal&&) noexcept                         = default;
        constexpr auto operator=(decimal const&) noexcept -> decimal& = default;
        constexpr auto operator=(decimal&&) noexcept -> decimal&      = default;

        [[nodiscard]] constexpr static auto precision() noexcept -> precision_type {
            return PrecisionV;
        }

        [[nodiscard]] constexpr static auto exponent() noexcept -> exponent_type {
            return -precision();
        }

        [[nodiscard]] constexpr static auto base() noexcept -> mantissa_type {
            return 10;
        }

        [[nodiscard]] constexpr static auto scale() noexcept -> mantissa_type {
            return fractional_scale<mantissa_type>(PrecisionV);
        }

        [[nodiscard]] constexpr static auto inverse_scale() noexcept -> floating_type {
            return 1.0 / scale();
        }

        [[nodiscard]] constexpr static auto zero() noexcept -> decimal {
            return from_shifted(0);
        }

        [[nodiscard]] constexpr static auto one() noexcept -> decimal {
            return from_unshifted(1.0);
        }

        [[nodiscard]] constexpr static auto infinity() noexcept -> decimal {
            return max();
        }

        [[nodiscard]] constexpr static auto nan() noexcept -> decimal {
            return max();
        }

        [[nodiscard]] constexpr static auto lowest() noexcept -> decimal {
            return from_shifted(std::numeric_limits<mantissa_type>::lowest());
        }

        [[nodiscard]] constexpr static auto min() noexcept -> decimal {
            return from_shifted(std::numeric_limits<mantissa_type>::min());
        }

        [[nodiscard]] constexpr static auto max() noexcept -> decimal {
            return from_shifted(std::numeric_limits<mantissa_type>::max());
        }

        template <std::integral IntegerT>
        [[nodiscard]] constexpr static auto from_shifted(IntegerT mantissa) noexcept -> decimal {
            return decimal{mantissa};
        }

        template <concepts::floating_point FloatingPointT>
        [[nodiscard]] constexpr static auto from_unshifted(FloatingPointT value) noexcept -> decimal {
            return decimal{value};
        }

        template <concepts::floating_point FloatingPointT>
        constexpr explicit decimal(FloatingPointT value) noexcept :
            mantissa_(static_cast<mantissa_type>(value * scale())) {}

        template <concepts::floating_point FloatingPointT>
        [[nodiscard]] constexpr auto unshifted() const noexcept -> FloatingPointT {
            return static_cast<FloatingPointT>(*this);
        }

        [[nodiscard]] constexpr auto shifted() const noexcept -> mantissa_type {
            return mantissa_;
        }

        [[nodiscard]] constexpr auto mantissa() const noexcept -> mantissa_type {
            return mantissa_;
        }

        template <concepts::floating_point FloatingPointT>
        [[nodiscard]] constexpr operator FloatingPointT() const noexcept {
            return unshifted<FloatingPointT>(mantissa());
        }

        [[nodiscard]] constexpr auto operator==(decimal rhs) const noexcept -> bool {
            return mantissa() == rhs.mantissa();
        }

        [[nodiscard]] constexpr auto operator!=(decimal rhs) const noexcept -> bool {
            return mantissa() != rhs.mantissa();
        }

        [[nodiscard]] constexpr auto operator<(decimal rhs) const noexcept -> bool {
            return mantissa() < rhs.mantissa();
        }

        [[nodiscard]] constexpr auto operator<=(decimal rhs) const noexcept -> bool {
            return mantissa() <= rhs.mantissa();
        }

        [[nodiscard]] constexpr auto operator>(decimal rhs) const noexcept -> bool {
            return mantissa() > rhs.mantissa();
        }

        [[nodiscard]] constexpr auto operator>=(decimal rhs) const noexcept -> bool {
            return mantissa() >= rhs.mantissa();
        }

        [[nodiscard]] constexpr auto operator-() const noexcept -> decimal {
            return from_shifted(-mantissa());
        }

        constexpr auto operator+=(decimal rhs) noexcept -> reference {
            mantissa_ += rhs.mantissa();
            return *this;
        }

        constexpr auto operator-=(decimal rhs) noexcept -> reference {
            mantissa_ -= rhs.mantissa();
            return *this;
        }

        constexpr auto operator*=(decimal rhs) noexcept -> reference {
            using wide_type       = __int128_t;
            auto const lhs_wide   = static_cast<wide_type>(mantissa());
            auto const rhs_wide   = static_cast<wide_type>(rhs.mantissa());
            auto const scale_wide = static_cast<wide_type>(scale());
            auto const result     = round_value<rounding_mode::truncate>((lhs_wide * rhs_wide), scale_wide);
            mantissa_             = static_cast<mantissa_type>(result);
            return *this;
        }

        constexpr auto operator/=(decimal rhs) noexcept -> reference {
            if (rhs == zero()) [[unlikely]] {
                return *this = decimal::infinity();
            }

            using wide_type     = __int128_t;
            auto const lhs_wide = static_cast<wide_type>(mantissa()) * scale();
            auto const rhs_wide = static_cast<wide_type>(rhs.mantissa());
            auto const result   = round_value<rounding_mode::truncate>(lhs_wide, rhs_wide);
            mantissa_           = static_cast<mantissa_type>(result);
            return *this;
        }
        template <concepts::floating_point FloatingPointT>
        constexpr auto operator+=(FloatingPointT rhs) noexcept -> reference {
            return *this += decimal{rhs};
        }

        template <concepts::floating_point FloatingPointT>
        constexpr auto operator-=(FloatingPointT rhs) noexcept -> reference {
            return *this -= decimal{rhs};
        }

        template <concepts::floating_point FloatingPointT>
        constexpr auto operator*=(FloatingPointT rhs) noexcept -> reference {
            (*this) *= decimal<PrecisionV, MantissaT>(rhs);
            return *this;
        }

        template <concepts::floating_point FloatingPointT>
        constexpr auto operator/=(FloatingPointT rhs) noexcept -> reference {
            (*this) /= decimal<PrecisionV, MantissaT>(rhs);
            return *this;
        }

        constexpr auto operator++() noexcept -> reference {
            ++mantissa_;
            return *this;
        }

        constexpr auto operator--() noexcept -> reference {
            --mantissa_;
            return *this;
        }

        constexpr auto operator++(int) noexcept -> decimal {
            auto copy = *this;
            ++*this;
            return copy;
        }

        constexpr auto operator--(int) noexcept -> decimal {
            auto copy = *this;
            --*this;
            return copy;
        }

        [[nodiscard]] constexpr static auto from_chars(std::string_view sv) noexcept -> decimal {
            auto const negative = not std::empty(sv) and sv[0] == '-';
            auto const factor   = static_cast<MantissaT>(1 - (negative << 1));
            auto const iterator = sv.find('.');
            if (iterator == std::string_view::npos) [[unlikely]] {
                auto const integral = from_chars<MantissaT>(sv);
                auto const shifted  = integral * scale();
                return from_shifted(shifted * factor);
            }

            auto const integral    = from_chars<MantissaT>(sv.substr(0, iterator));
            auto const fractional  = sv.substr(iterator + 1);
            auto const limit       = std::min<std::size_t>(std::size(fractional), PrecisionV);
            auto const floating    = from_chars<MantissaT>(fractional.substr(0, limit));
            auto const fract_scale = fractional_scale<MantissaT>(PrecisionV - limit);
            auto const shifted     = integral * scale() + floating * fract_scale;
            return from_shifted(shifted * factor);
        }

        [[nodiscard]] constexpr static auto from(std::string_view data) noexcept -> decimal {
            return decimal::from_chars(data);
        }

        template <concepts::floating_point FloatingPointT>
        [[nodiscard]] constexpr static auto from(FloatingPointT from) noexcept -> decimal {
            return decimal::from_unshifted(from);
        }

        template <std::integral IntegralT>
        [[nodiscard]] constexpr static auto from(IntegralT from) noexcept -> decimal {
            return decimal::from_shifted(from);
        }

        template <std::int8_t Precision2V, typename Mantissa2T>
        [[nodiscard]] constexpr static auto from(decimal<Precision2V, Mantissa2T> from) noexcept -> decimal {
            constexpr static auto delta = Precision2V - precision();
            if constexpr (delta == 0) {
                return from_shifted(from.shifted());
            } else if constexpr (delta > 0) {
                return from_shifted(from.shifted() * fractional_scale<mantissa_type>(delta));
            } else {
                return from_shifted(from.shifted() / fractional_scale<mantissa_type>(-delta));
            }
        }

        template <std::int8_t Precision2V>
        [[nodiscard]] constexpr auto to() const noexcept -> decimal<Precision2V, MantissaT> {
            constexpr static auto delta = Precision2V - precision();
            if constexpr (delta == 0) {
                return decimal<Precision2V, MantissaT>::from_shifted(shifted());
            } else if constexpr (delta > 0) {
                return decimal<Precision2V, MantissaT>::from_shifted(shifted() * fractional_scale<MantissaT>(delta));
            } else {
                return decimal<Precision2V, MantissaT>::from_shifted(shifted() / fractional_scale<MantissaT>(-delta));
            }
        }

        template <typename OtherDecimalT>
        [[nodiscard]] constexpr auto to() const noexcept -> decimal<OtherDecimalT::precision(), MantissaT> {
            return to<OtherDecimalT::precision()>();
        }

        template <decimal_format FormatV>
        [[nodiscard]] constexpr auto to_chars(char* buffer) const noexcept -> char* {
            auto const negative   = mantissa() < 0;
            auto const value      = negative ? -mantissa() : mantissa();
            auto const integral   = value / scale();
            auto const fractional = value % scale();
            *buffer               = '-';
            buffer                = to_chars(std::next(buffer, negative), integral);
            *buffer               = '.';
            auto const start      = ++buffer;
            auto const digits     = count(fractional);
            auto const padding    = precision() - digits;
            std::fill(start, start + padding, '0');
            buffer = to_chars(start + padding, fractional);
            if constexpr (FormatV == decimal_format::trimmed) {
                while (buffer > start + 1 && *(buffer - 1) == '0') {
                    --buffer;
                }

                if (buffer == start) {
                    *buffer++ = '0';
                }
            }
            return buffer;
        }

        template <decimal_format FormatV>
        [[nodiscard]] constexpr auto to_string() const noexcept -> string_type {
            auto buffer = std::array<char, 64>{};
            auto ending = to_chars<FormatV>(std::data(buffer));
            return string_type{std::data(buffer), ending};
        }

        [[nodiscard]] constexpr auto to_string() const noexcept -> string_type {
            return to_string<decimal_format::trimmed>();
        }

        [[nodiscard]] constexpr auto trimmed() const noexcept -> string_type {
            return to_string<decimal_format::trimmed>();
        }

        [[nodiscard]] constexpr auto padded() const noexcept -> string_type {
            return to_string<decimal_format::padded>();
        }

        [[nodiscard]] constexpr auto is_negative() const noexcept -> bool {
            return mantissa() < 0;
        }

        [[nodiscard]] constexpr auto is_positive() const noexcept -> bool {
            return mantissa() > 0;
        }

        [[nodiscard]] constexpr auto is_zero() const noexcept -> bool {
            return mantissa() == 0;
        }

        [[nodiscard]] constexpr auto is_integer() const noexcept -> bool {
            return mantissa() % scale() == 0;
        }

        [[nodiscard]] constexpr auto is_finite() const noexcept -> bool {
            return ((*this) != min()) and ((*this) != max());
        }

        [[nodiscard]] constexpr auto is_nan() const noexcept -> bool {
            return ((*this) == nan());
        }

        [[nodiscard]] constexpr auto negate() const noexcept -> decimal {
            return decimal::from_shifted(-mantissa());
        }

        [[nodiscard]] constexpr auto abs() const noexcept -> decimal {
            return decimal::from_shifted(is_negative() ? -mantissa() : mantissa());
        }

        [[nodiscard]] constexpr auto sign() const noexcept -> std::int8_t {
            return is_negative() ? -1 : (is_positive() ? 1 : 0);
        }

    private:
        template <std::integral IntegerT>
        constexpr explicit decimal(IntegerT value) noexcept : mantissa_(static_cast<mantissa_type>(value)) {}

        template <concepts::floating_point FloatingPointT>
        [[nodiscard]] constexpr static auto unshifted(mantissa_type shifted) noexcept -> FloatingPointT {
            return static_cast<FloatingPointT>(shifted * inverse_scale());
        }

        template <std::integral IntegralT>
        [[nodiscard]] constexpr static auto from_chars(std::string_view sv) noexcept -> IntegralT {
            auto const negative = not std::empty(sv) and sv[0] == '-';
            auto begin          = std::next(std::begin(sv), negative);
            auto end            = std::end(sv);
            auto result         = IntegralT{0};
            for (; begin != end; begin = std::next(begin)) {
                SPL_ASSERT(*begin >= '0' && *begin <= '9');
                result = result * 10 + (*begin - '0');
            }
            return result;
        }

        template <std::integral IntegralT>
        [[nodiscard]] constexpr auto to_chars(char* buffer, IntegralT value) const noexcept -> char* {
            constexpr static char const pairs[] = {"00010203040506070809"
                                                   "10111213141516171819"
                                                   "20212223242526272829"
                                                   "30313233343536373839"
                                                   "40414243444546474849"
                                                   "50515253545556575859"
                                                   "60616263646566676869"
                                                   "70717273747576777879"
                                                   "80818283848586878889"
                                                   "90919293949596979899"};

            auto* ptr = buffer;
            while (value >= 100) {
                auto const i = (value % 100) * 2;
                value /= 100;
                *ptr++ = pairs[i + 1];
                *ptr++ = pairs[i];
            }

            if (value < 10) {
                *ptr++ = static_cast<char>('0' + value);
            } else {
                auto const i = value * 2;
                *ptr++       = pairs[i + 1];
                *ptr++       = pairs[i];
            }

            std::reverse(buffer, ptr);
            return ptr;
        }

        template <std::integral IntegralT>
        [[nodiscard]] constexpr static auto fractional_scale(std::size_t n) noexcept -> IntegralT {
            constexpr static IntegralT table[] = {1,
                                                  10,
                                                  100,
                                                  1'000,
                                                  10'000,
                                                  100'000,
                                                  1'000'000,
                                                  10'000'000,
                                                  100'000'000,
                                                  1'000'000'000,
                                                  10'000'000'000,
                                                  100'000'000'000,
                                                  1'000'000'000'000,
                                                  10'000'000'000'000,
                                                  100'000'000'000'000,
                                                  1'000'000'000'000'000,
                                                  10'000'000'000'000'000};
            return table[n];
        }

        template <std::integral IntegralT>
        [[nodiscard]] constexpr static auto count(IntegralT value) noexcept -> std::size_t {
            std::size_t count = 1;
            while (value >= 10) {
                value /= 10;
                ++count;
            }
            return count;
        }

        enum class rounding_mode { half_up, half_down, half_even, truncate, ceiling, floor };

        template <rounding_mode RoundingModeV, typename NumericT>
        [[nodiscard]] constexpr auto round_value(NumericT value, NumericT scale) noexcept -> NumericT {
            if constexpr (RoundingModeV == rounding_mode::half_up) {
                return (value + (scale / 2)) / scale;
            } else if constexpr (RoundingModeV == rounding_mode::half_down) {
                return (value - (scale / 2)) / scale;
            } else if constexpr (RoundingModeV == rounding_mode::half_even) {
                auto const rounded = (value + scale / 2) / scale;
                return (rounded % 2 == 0) ? rounded : rounded + 1;
            } else if constexpr (RoundingModeV == rounding_mode::truncate) {
                return value / scale;
            } else if constexpr (RoundingModeV == rounding_mode::ceiling) {
                return (value + scale - 1) / scale;
            } else if constexpr (RoundingModeV == rounding_mode::floor) {
                return value / scale;
            }
        }

        mantissa_type mantissa_{0};
    };

    template <std::int16_t PrecisionV, typename MantissaT, concepts::floating_point FloatingPointT>
    [[nodiscard]] constexpr auto operator==(FloatingPointT lhs, decimal<PrecisionV, MantissaT> rhs) noexcept -> bool {
        return decimal<PrecisionV, MantissaT>{lhs} == rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT, concepts::floating_point FloatingPointT>
    [[nodiscard]] constexpr auto operator!=(FloatingPointT lhs, decimal<PrecisionV, MantissaT> rhs) noexcept -> bool {
        return decimal<PrecisionV, MantissaT>{lhs} != rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT, concepts::floating_point FloatingPointT>
    [[nodiscard]] constexpr auto operator<(FloatingPointT lhs, decimal<PrecisionV, MantissaT> rhs) noexcept -> bool {
        return decimal<PrecisionV, MantissaT>{lhs} < rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT, concepts::floating_point FloatingPointT>
    [[nodiscard]] constexpr auto operator<=(FloatingPointT lhs, decimal<PrecisionV, MantissaT> rhs) noexcept -> bool {
        return decimal<PrecisionV, MantissaT>{lhs} <= rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT, concepts::floating_point FloatingPointT>
    [[nodiscard]] constexpr auto operator>(FloatingPointT lhs, decimal<PrecisionV, MantissaT> rhs) noexcept -> bool {
        return decimal<PrecisionV, MantissaT>{lhs} > rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT, concepts::floating_point FloatingPointT>
    [[nodiscard]] constexpr auto operator>=(FloatingPointT lhs, decimal<PrecisionV, MantissaT> rhs) noexcept -> bool {
        return decimal<PrecisionV, MantissaT>{lhs} >= rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT>
    [[nodiscard]] constexpr auto operator+(decimal<PrecisionV, MantissaT> lhs,
                                           decimal<PrecisionV, MantissaT> rhs) noexcept
        -> decimal<PrecisionV, MantissaT> {
        return lhs += rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT>
    [[nodiscard]] constexpr auto operator-(decimal<PrecisionV, MantissaT> lhs,
                                           decimal<PrecisionV, MantissaT> rhs) noexcept
        -> decimal<PrecisionV, MantissaT> {
        return lhs -= rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT>
    [[nodiscard]] constexpr auto operator*(decimal<PrecisionV, MantissaT> lhs,
                                           decimal<PrecisionV, MantissaT> rhs) noexcept
        -> decimal<PrecisionV, MantissaT> {
        return lhs *= rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT>
    [[nodiscard]] constexpr auto operator/(decimal<PrecisionV, MantissaT> lhs,
                                           decimal<PrecisionV, MantissaT> rhs) noexcept
        -> decimal<PrecisionV, MantissaT> {
        return lhs /= rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT, concepts::floating_point FloatingPointT>
    [[nodiscard]] constexpr auto operator+(decimal<PrecisionV, MantissaT> lhs, FloatingPointT rhs) noexcept
        -> decimal<PrecisionV, MantissaT> {
        return lhs += rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT, concepts::floating_point FloatingPointT>
    [[nodiscard]] constexpr auto operator+(FloatingPointT lhs, decimal<PrecisionV, MantissaT> rhs) noexcept
        -> decimal<PrecisionV, MantissaT> {
        return decimal<PrecisionV, MantissaT>{lhs} + rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT, concepts::floating_point FloatingPointT>
    [[nodiscard]] constexpr auto operator-(decimal<PrecisionV, MantissaT> lhs, FloatingPointT rhs) noexcept
        -> decimal<PrecisionV, MantissaT> {
        return lhs -= rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT, concepts::floating_point FloatingPointT>
    [[nodiscard]] constexpr auto operator-(FloatingPointT lhs, decimal<PrecisionV, MantissaT> rhs) noexcept
        -> decimal<PrecisionV, MantissaT> {
        return decimal<PrecisionV, MantissaT>{lhs} - rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT, concepts::floating_point FloatingPointT>
    [[nodiscard]] constexpr auto operator*(decimal<PrecisionV, MantissaT> lhs, FloatingPointT rhs) noexcept
        -> decimal<PrecisionV, MantissaT> {
        return lhs *= rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT, concepts::floating_point FloatingPointT>
    [[nodiscard]] constexpr auto operator*(FloatingPointT lhs, decimal<PrecisionV, MantissaT> rhs) noexcept
        -> decimal<PrecisionV, MantissaT> {
        return decimal<PrecisionV, MantissaT>{lhs} * rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT, concepts::floating_point FloatingPointT>
    [[nodiscard]] constexpr auto operator/(decimal<PrecisionV, MantissaT> lhs, FloatingPointT rhs) noexcept
        -> decimal<PrecisionV, MantissaT> {
        return lhs /= rhs;
    }

    template <std::int16_t PrecisionV, typename MantissaT, concepts::floating_point FloatingPointT>
    [[nodiscard]] constexpr auto operator/(FloatingPointT lhs, decimal<PrecisionV, MantissaT> rhs) noexcept
        -> decimal<PrecisionV, MantissaT> {
        return decimal<PrecisionV, MantissaT>{lhs} / rhs;
    }

} // namespace spl::types

template <std::int16_t PrecisionV, typename MantissaT>
struct std::numeric_limits<spl::types::decimal<PrecisionV, MantissaT>> {
    [[nodiscard]] constexpr static auto min() noexcept -> spl::types::decimal<PrecisionV, MantissaT> {
        return spl::types::decimal<PrecisionV, MantissaT>::min();
    }

    [[nodiscard]] constexpr static auto max() noexcept -> spl::types::decimal<PrecisionV, MantissaT> {
        return spl::types::decimal<PrecisionV, MantissaT>::max();
    }

    [[nodiscard]] constexpr static auto lowest() noexcept -> spl::types::decimal<PrecisionV, MantissaT> {
        return spl::types::decimal<PrecisionV, MantissaT>::lowest();
    }

    [[nodiscard]] constexpr static auto quiet_NaN() noexcept -> spl::types::decimal<PrecisionV, MantissaT> {
        return spl::types::decimal<PrecisionV, MantissaT>::max();
    }

    [[nodiscard]] constexpr static auto infinity() noexcept -> spl::types::decimal<PrecisionV, MantissaT> {
        return spl::types::decimal<PrecisionV, MantissaT>::max();
    }

    [[nodiscard]] constexpr static auto epsilon() noexcept -> spl::types::decimal<PrecisionV, MantissaT> {
        return spl::types::decimal<PrecisionV, MantissaT>::from_shifted(1);
    }

    [[nodiscard]] constexpr static auto round_error() noexcept -> spl::types::decimal<PrecisionV, MantissaT> {
        return spl::types::decimal<PrecisionV, MantissaT>::from_shifted(0);
    }

    [[nodiscard]] constexpr static auto is_iec559() noexcept -> bool {
        return false;
    }

    [[nodiscard]] constexpr static auto digits() noexcept -> int {
        return std::numeric_limits<MantissaT>::digits;
    }

    [[nodiscard]] constexpr static auto digits10() noexcept -> int {
        return std::numeric_limits<MantissaT>::digits10;
    }

    [[nodiscard]] constexpr static auto max_exponent() noexcept -> int {
        return PrecisionV;
    }

    [[nodiscard]] constexpr static auto min_exponent() noexcept -> int {
        return -PrecisionV;
    }

    constexpr static auto is_specialized = true;
    constexpr static auto is_signed      = true;
    constexpr static auto is_integer     = false;
    constexpr static auto is_exact       = false;
};

template <std::int16_t PrecisionV, typename MantissaT>
struct std::hash<spl::types::decimal<PrecisionV, MantissaT>> {
    [[nodiscard]] constexpr auto operator()(spl::types::decimal<PrecisionV, MantissaT> value) const noexcept
        -> std::uint64_t {
        return std::hash<MantissaT>{}(value.shifted());
    }
};
