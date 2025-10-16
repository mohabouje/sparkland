#pragma once

namespace spl { inline namespace meta {

    class nonmovable {
    public:
        constexpr nonmovable() = default;

        constexpr ~nonmovable() = default;

        constexpr nonmovable(nonmovable&&) = delete;

        constexpr auto operator=(nonmovable&&) -> nonmovable& = delete;
    };

}} // namespace spl::meta
