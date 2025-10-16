#pragma once

#include <optional>
#include <type_traits>

namespace spl::concepts {

    namespace {

        template <typename TypeT>
        struct is_optional_implementation : std::false_type {};

        template <typename TypeT>
        struct is_optional_implementation<std::optional<TypeT>> : std::true_type {};

    } // namespace

    template <typename TypeT>
    concept optional = is_optional_implementation<std::decay_t<TypeT>>::value;

} // namespace spl::concepts
