#pragma once

namespace spl { inline namespace meta {

    class noncopyable {
    public:
        constexpr noncopyable() = default;

        constexpr ~noncopyable() = default;

        constexpr noncopyable(noncopyable const&) = delete;

        constexpr auto operator=(noncopyable const&) -> noncopyable& = delete;
    };

}} // namespace spl::meta
