#pragma once

#include <string_view>

namespace spl { inline namespace core {

    template <typename... ArgsT>
    constexpr void unused(ArgsT&&...) noexcept {}

    template <typename T>
    constexpr void unused(T&&, std::string_view) noexcept {}

}} // namespace spl::core
