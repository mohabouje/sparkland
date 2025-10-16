#pragma once

namespace spl::inline meta {

    template <auto ValueV>
    struct typed {
        constexpr static auto value = ValueV;
    };

} // namespace spl::inline meta