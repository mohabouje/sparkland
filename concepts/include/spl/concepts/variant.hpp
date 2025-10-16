#pragma once

#include <concepts>
#include <variant>

namespace spl::concepts {

    template <typename T>
    struct is_variant : std::false_type {};

    template <typename... Args>
    struct is_variant<std::variant<Args...>> : std::true_type {};

    template <typename T>
    concept variant = is_variant<std::decay_t<T>>::value;

} // namespace spl::concepts
