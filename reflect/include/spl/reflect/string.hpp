#pragma once

#include "spl/concepts/types.hpp"
#include "spl/meta/always_false.hpp"

#include "spl/reflect/object.hpp"
#include "spl/reflect/enum.hpp"

namespace spl::reflect {

    template <typename TypeT, spl::concepts::string StringT>
    requires(not spl::concepts::scoped_enum<TypeT>)
    constexpr auto from_string(StringT&& string, TypeT& value) noexcept -> void {
        static_assert(spl::meta::always_false<TypeT>(), "not implemented yet");
    }

    template <typename TypeT, spl::concepts::string StringT>
    requires(not spl::concepts::scoped_enum<TypeT>)
    constexpr auto to_string(TypeT&& value, StringT& buffer) noexcept -> void {
        buffer = std::format("{}", std::forward<TypeT>(value));
    }

    template <spl::concepts::scoped_enum TypeT, spl::concepts::string StringT>
    constexpr auto from_string(StringT&& string, TypeT& value) noexcept -> void {
        value = spl::reflect::enum_from_string<TypeT>(std::forward<StringT>(string));
    }

    template <spl::concepts::scoped_enum TypeT, spl::concepts::string StringT>
    constexpr auto to_string(TypeT&& value, StringT& buffer) noexcept -> void {
        buffer = spl::reflect::enum_to_string(value);
    }

    template <typename TypeT, typename StringT = std::string>
    requires(not spl::concepts::scoped_enum<TypeT>)
    [[nodiscard]] inline auto to_string(TypeT&& value) noexcept -> StringT {
        StringT temporal;
        to_string(std::forward<TypeT>(value), temporal);
        return temporal;
    }

    template <concepts::scoped_enum EnumT>
    [[nodiscard]] constexpr auto to_string(EnumT&& value) noexcept -> std::string_view {
        return spl::reflect::enum_to_string(value);
    }

    template <typename TypeT, spl::concepts::string StringT>
    [[nodiscard]] constexpr auto from_string(StringT&& string) noexcept -> TypeT {
        TypeT temporal;
        from_string(std::forward<StringT>(string), temporal);
        return temporal;
    }
} // namespace spl::reflect
