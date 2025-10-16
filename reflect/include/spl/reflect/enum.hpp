#pragma once

#include "spl/concepts/types.hpp"

#include <magic_enum/magic_enum_all.hpp>

#include <limits>
#include <type_traits>

namespace spl::reflect {

    template <concepts::scoped_enum EnumT>
    [[nodiscard]] constexpr auto enum_to_string(EnumT value) noexcept -> std::string_view {
        return magic_enum::enum_name(value);
    }

    template <concepts::scoped_enum EnumT>
    [[nodiscard]] constexpr auto enum_from_string(std::string_view name) noexcept -> EnumT {
        return magic_enum::enum_cast<EnumT>(name).value_or(
            EnumT{std::numeric_limits<std::underlying_type_t<EnumT>>::infinity()});
    }

    template <concepts::scoped_enum EnumT>
    [[nodiscard]] constexpr auto to_index(EnumT value) noexcept -> std::size_t {
        return magic_enum::enum_index(value).value_or(std::numeric_limits<std::underlying_type_t<EnumT>>::infinity());
    }

    template <concepts::scoped_enum EnumT>
    [[nodiscard]] constexpr auto from_index(std::size_t index) noexcept -> EnumT {
        auto const alternatives = std::array{EnumT{std::numeric_limits<std::underlying_type_t<EnumT>>::infinity()},
                                             magic_enum::enum_values<EnumT>()[index % magic_enum::enum_count<EnumT>()]};
        return alternatives[index < magic_enum::enum_count<EnumT>()];
    }

    template <concepts::scoped_enum EnumT>
    [[nodiscard]] constexpr auto to_underlying(EnumT value) noexcept -> std::underlying_type_t<EnumT> {
        return std::to_underlying(value);
    }

    template <concepts::scoped_enum EnumT>
    [[nodiscard]] constexpr auto from_underlying(std::underlying_type_t<EnumT> underlying) noexcept -> EnumT {
        return magic_enum::enum_cast<EnumT>(underlying)
            .value_or(EnumT{std::numeric_limits<std::underlying_type_t<EnumT>>::infinity()});
        ;
    }

    template <concepts::scoped_enum EnumT>
    [[nodiscard]] constexpr auto contains(std::underlying_type_t<EnumT> underlying) noexcept -> bool {
        return magic_enum::enum_contains(EnumT(underlying));
    }

    template <concepts::scoped_enum EnumT>
    [[nodiscard]] constexpr auto contains(EnumT value) noexcept -> bool {
        return magic_enum::enum_contains(value);
    }

    template <concepts::scoped_enum EnumT>
    [[nodiscard]] constexpr auto contains(std::string_view value) noexcept -> bool {
        auto const temporal = magic_enum::enum_cast<EnumT>(value);
        return temporal.has_value() and magic_enum::enum_contains(temporal.value());
    }

    template <concepts::scoped_enum EnumT>
    [[nodiscard]] constexpr auto size() noexcept -> std::size_t {
        return magic_enum::enum_count<EnumT>();
    }

    template <concepts::scoped_enum EnumT>
    [[nodiscard]] constexpr auto count() noexcept -> std::size_t {
        return magic_enum::enum_count<EnumT>();
    }

    template <concepts::scoped_enum EnumT>
    [[nodiscard]] consteval auto names() noexcept -> auto const& {
        return magic_enum::enum_names<EnumT>();
    }

    template <concepts::scoped_enum EnumT>
    [[nodiscard]] consteval auto values() noexcept -> auto const& {
        return magic_enum::enum_values<EnumT>();
    }

    template <concepts::scoped_enum EnumT>
    [[nodiscard]] consteval auto entries() noexcept -> auto const& {
        return magic_enum::enum_entries<EnumT>();
    }

    template <concepts::scoped_enum EnumT, typename FunctorT>
    constexpr auto for_each(FunctorT&& functor) noexcept(std::is_nothrow_invocable_v<FunctorT>) -> void {
        magic_enum::enum_for_each<EnumT>(std::forward<FunctorT>(functor));
    }

} // namespace spl::reflect
