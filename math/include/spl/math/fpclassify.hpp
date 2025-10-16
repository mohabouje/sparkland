#pragma once

#include "spl/concepts/types.hpp"
#include "spl/types/strong.hpp"

#include <boost/math/special_functions/fpclassify.hpp>

namespace spl::math {

    enum class fp_class {
        fp_nan       = FP_NAN,
        fp_infinite  = FP_INFINITE,
        fp_zero      = FP_ZERO,
        fp_normal    = FP_NORMAL,
        fp_subnormal = FP_SUBNORMAL,
#if defined(__APPLE__) && defined(FP_SUPERNORMAL)
        fp_supernormal = FP_SUPERNORMAL,
#endif
    };

    template <spl::concepts::floating_point FloatT, typename... RestT>
    [[nodiscard]] constexpr auto fpclassify(spl::types::strong::type<FloatT, RestT...> value) noexcept -> fp_class {
        return fpclassify(value.value_of());
    }

    template <spl::concepts::floating_point FloatT>
    [[nodiscard]] constexpr auto isfinite(FloatT value) noexcept -> bool {
        return boost::math::isfinite(value);
    }

    template <spl::concepts::floating_point FloatT>
    [[nodiscard]] constexpr auto isnan(FloatT value) noexcept -> bool {
        return boost::math::isnan(value);
    }

    template <spl::concepts::floating_point FloatT>
    [[nodiscard]] constexpr auto isinf(FloatT value) noexcept -> bool {
        return boost::math::isinf(value);
    }

    template <spl::concepts::floating_point FloatT>
    [[nodiscard]] constexpr auto isnormal(FloatT value) noexcept -> bool {
        return boost::math::isnormal(value);
    }

    template <spl::concepts::floating_point FloatT, typename... RestT>
    [[nodiscard]] constexpr auto isinf(spl::types::strong::type<FloatT, RestT...> value) noexcept -> bool {
        return isinf(value.value_of());
    }

    template <spl::concepts::floating_point FloatT, typename... RestT>
    [[nodiscard]] constexpr auto isnan(spl::types::strong::type<FloatT, RestT...> value) noexcept -> bool {
        return isnan(value.value_of());
    }

    template <spl::concepts::floating_point FloatT, typename... RestT>
    [[nodiscard]] constexpr auto isfinite(spl::types::strong::type<FloatT, RestT...> value) noexcept -> bool {
        return isfinite(value.value_of());
    }

    template <spl::concepts::floating_point FloatT, typename... RestT>
    [[nodiscard]] constexpr auto isnormal(spl::types::strong::type<FloatT, RestT...> value) noexcept -> bool {
        return isnormal(value.value_of());
    }

} // namespace spl::math