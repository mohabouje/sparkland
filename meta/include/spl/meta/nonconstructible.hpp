#pragma once

namespace spl { inline namespace meta {

    class nonconstructible {
    public:
        constexpr nonconstructible() = delete;

        constexpr ~nonconstructible() = delete;

        constexpr nonconstructible(nonconstructible const&) = delete;

        constexpr nonconstructible(nonconstructible&&) = delete;

        constexpr auto operator=(nonconstructible const&) -> nonconstructible& = delete;

        constexpr auto operator=(nonconstructible&) -> nonconstructible& = delete;
    };

}} // namespace spl::meta
