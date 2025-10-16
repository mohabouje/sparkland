#pragma once

#include "spl/concepts/types.hpp"
#include "spl/concepts/optional.hpp"
#include "spl/concepts/object.hpp"
#include "spl/meta/typeinfo.hpp"
#include "spl/meta/variant.hpp"
#include "spl/types/decimal.hpp"
#include "spl/reflect/object.hpp"
#include "spl/reflect/enum.hpp"

#include <boost/lexical_cast.hpp>

#include <chrono>
#include <sstream>
#include <filesystem>

template <spl::concepts::optional OptionalT>
struct std::formatter<OptionalT> : public std::formatter<std::string_view> {
    template <typename FormatContext>
    [[nodiscard]] constexpr auto format(OptionalT const& value, FormatContext&& ctx) const noexcept
        -> std::format_context::iterator {
        if (not value.has_value()) {
            return std::format_to(ctx.out(), "nullopt");
        }
        return std::format_to(ctx.out(), "{}", value.value());
    }
};

template <spl::concepts::object ObjectT>
requires(not std::is_same_v<ObjectT, std::tm>)
struct std::formatter<ObjectT> : public std::formatter<std::string_view> {
    template <typename InstanceT, typename FormatContext>
    [[nodiscard]] constexpr auto format(InstanceT&& value, FormatContext&& ctx) const noexcept
        -> std::format_context::iterator {
        auto buffer = std::stringstream{};
        buffer << spl::meta::type<ObjectT>();
        buffer << '{';
        spl::reflect::for_each(std::forward<InstanceT>(value),
                               [&buffer, index = std::size_t{0}]<typename FieldT>(auto&& name, FieldT&& field) mutable {
                                   buffer << std::forward<decltype(name)>(name);
                                   buffer << '=';
                                   buffer << std::format("{}", std::forward<FieldT>(field));
                                   if (++index != spl::reflect::count<ObjectT>()) [[likely]] {
                                       buffer << ',';
                                   }
                               });
        buffer << '}';
        return std::formatter<std::string_view>::format(buffer.view(), std::forward<FormatContext>(ctx));
    }
};

// template <spl::concepts::scoped_enum EnumT>
// struct std::formatter<EnumT> : public std::formatter<std::string_view> {
//     template <typename InstanceT, typename FormatContext>
//     [[nodiscard]] constexpr auto format(InstanceT&& value, FormatContext&& ctx) const noexcept
//         -> std::format_context::iterator {
//         return std::formatter<std::string_view>::format(spl::reflect::enum_to_string(value),
//                                                         std::forward<FormatContext>(ctx));
//     }
// };

template <>
struct std::formatter<std::chrono::month> : public std::formatter<unsigned int> {
    template <typename FormatContext>
    [[nodiscard]] constexpr auto format(std::chrono::month const& value, FormatContext&& ctx) const noexcept
        -> std::format_context::iterator {
        auto const underlying = static_cast<unsigned>(value);
        return std::formatter<unsigned int>::format(underlying, std::forward<FormatContext>(ctx));
    }
};

template <>
struct std::formatter<std::chrono::day> : public std::formatter<unsigned int> {
    template <typename FormatContext>
    [[nodiscard]] constexpr auto format(std::chrono::day const& value, FormatContext&& ctx) const noexcept
        -> std::format_context::iterator {
        auto const underlying = static_cast<unsigned>(value);
        return std::formatter<unsigned int>::format(underlying, std::forward<FormatContext>(ctx));
    }
};

template <>
struct std::formatter<std::chrono::year> : public std::formatter<int> {
    template <typename FormatContext>
    [[nodiscard]] constexpr auto format(std::chrono::year const& value, FormatContext&& ctx) const noexcept
        -> std::format_context::iterator {
        auto const underlying = static_cast<int>(value);
        return std::formatter<int>::format(underlying, std::forward<FormatContext>(ctx));
    }
};

template <>
struct std::formatter<std::chrono::year_month_day> : public std::formatter<std::string_view> {
    template <typename FormatContext>
    [[nodiscard]] constexpr auto format(std::chrono::year_month_day const& value, FormatContext&& ctx) const noexcept
        -> std::format_context::iterator {
        return std::format_to(ctx.out(), "{}/{}/{}", value.year(), value.month(), value.day());
    }
};

template <>
struct std::formatter<std::filesystem::path> : public std::formatter<std::string_view> {
    template <typename FormatContext>
    [[nodiscard]] constexpr auto format(std::filesystem::path const& value, FormatContext&& ctx) const noexcept
        -> std::format_context::iterator {
        return std::format_to(ctx.out(), "{}", value.string());
    }
};

template <std::ranges::range RangeT>
requires(not spl::concepts::string<RangeT>)
struct std::formatter<RangeT> : public std::formatter<std::string_view> {
    template <typename FormatContext>
    [[nodiscard]] constexpr auto format(RangeT const& value, FormatContext&& ctx) const noexcept
        -> std::format_context::iterator {
        auto buffer = std::stringstream{};
        buffer << '[';
        for (auto i = std::cbegin(value); i != std::cend(value); ++i) {
            buffer << std::format("{}", *i);
            if (std::next(i) != std::cend(value)) {
                buffer << ',';
            }
        }
        buffer << ']';
        return std::formatter<std::string_view>::format(buffer.view(), std::forward<FormatContext>(ctx));
    }
};

template <typename... TypeT>
struct std::formatter<std::variant<TypeT...>> : public std::formatter<std::string_view> {
    template <typename FormatContext>
    [[nodiscard]] constexpr auto format(std::variant<TypeT...> const& value, FormatContext&& ctx) const noexcept
        -> std::format_context::iterator {
        return spl::fast_visit([&](auto&& item) { return std::format_to(ctx.out(), "variant{{{}}}", item); }, value);
    }
};

template <typename... TypeT>
struct std::formatter<std::tuple<TypeT...>> : public std::formatter<std::string_view> {
    template <typename FormatContext, std::size_t... IndexV>
    [[nodiscard]] constexpr auto format(std::tuple<TypeT...> const& value, FormatContext&& ctx,
                                        std::index_sequence<IndexV...>) const noexcept
        -> std::format_context::iterator {
        auto buffer = std::stringstream();
        buffer << '(';
        ((buffer << std::format("{}{}", std::get<IndexV>(value), IndexV != (sizeof...(TypeT) - 1) ? ", " : "")), ...);
        buffer << ')';
        return std::formatter<std::string_view>::format(buffer.view(), std::forward<FormatContext>(ctx));
    }

    template <typename FormatContext>
    [[nodiscard]] constexpr auto format(std::tuple<TypeT...> const& value, FormatContext&& ctx) const noexcept
        -> std::format_context::iterator {
        return format(value, ctx, std::make_index_sequence<sizeof...(TypeT)>{});
    }
};

template <typename FirstT, typename SecondT>
struct std::formatter<std::pair<FirstT, SecondT>> : public std::formatter<std::string_view> {
    template <typename FormatContext>
    [[nodiscard]] constexpr auto format(std::pair<FirstT, SecondT> const& value, FormatContext&& ctx) const noexcept
        -> std::format_context::iterator {
        return std::format_to(ctx.out(), "({}, {})", value.first, value.second);
    }
};

template <std::int16_t PrecisionV, typename MantissaT>
struct std::formatter<spl::types::decimal<PrecisionV, MantissaT>> : public std::formatter<std::string_view> {
    template <typename FormatContext>
    [[nodiscard]] constexpr auto format(spl::types::decimal<PrecisionV, MantissaT> value,
                                        FormatContext&& ctx) const noexcept -> std::format_context::iterator {
        auto const trimmed = value.trimmed();
        return std::format_to(ctx.out(), "{}", trimmed);
    }
};