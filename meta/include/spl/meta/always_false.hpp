#pragma once

namespace spl { inline namespace meta {

    template <typename...>
    struct always_false {
        constexpr static bool value = false;
    };

}} // namespace spl::meta
