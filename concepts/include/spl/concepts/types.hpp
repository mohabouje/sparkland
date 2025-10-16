#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <ranges>

namespace spl::concepts {

    template <typename BooleanT>
    concept boolean = std::same_as<BooleanT, bool> and std::integral<BooleanT>;

    template <typename T>
    concept byte = std::same_as<T, std::byte>;

    template <typename NumericT>
    concept arithmetic = std::is_arithmetic_v<NumericT>;

    template <typename NumericT>
    concept strictly_numeric = std::is_arithmetic_v<NumericT> and not concepts::boolean<NumericT>;

    template <typename NumericT>
    concept strictly_integral = std::is_integral_v<NumericT> and strictly_numeric<NumericT>;

    template <typename IntegralT>
    concept unsigned_integral = strictly_integral<IntegralT> and std::is_unsigned_v<IntegralT>;

    template <typename IntegralT>
    concept signed_integral = strictly_integral<IntegralT> and std::is_signed_v<IntegralT>;

    template <typename FloatingT>
    concept floating_point = strictly_numeric<FloatingT> and std::floating_point<FloatingT>;

    template <typename StringT>
    concept string = std::ranges::range<StringT> and std::is_same_v<std::ranges::range_value_t<StringT>, char>;

    template <typename EnumT>
    concept scoped_enum = std::is_scoped_enum_v<std::decay_t<EnumT>>;

    template <typename StringT>
    concept cstring = std::is_same_v<char*, StringT> or std::is_same_v<char const*, StringT>;

    template <typename T>
    concept byte_like = byte<std::decay_t<T>> or std::is_same_v<std::decay_t<T>, unsigned char> or
                        std::is_same_v<std::decay_t<T>, std::byte>;

} // namespace spl::concepts
