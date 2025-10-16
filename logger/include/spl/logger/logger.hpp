#pragma once

#include "spl/reflect/format.hpp"
#include "spl/logger/level.hpp"
#include "spl/logger/color.hpp"

#include <format>
#include <print>
#include <source_location>
#include <chrono>
#include <utility>

namespace spl::logger {

    template <logger::level LevelV, typename... ArgsT>
    struct dispatcher {
        constexpr dispatcher(std::source_location const source_location, std::format_string<ArgsT...> const& format,
                             ArgsT&&... args) noexcept {
#ifdef NDEBUG
            if constexpr (LevelV <= level::debug) {
                return;
            }
#endif

            auto const message   = std::format(format, std::forward<ArgsT>(args)...);
            auto const level     = LevelV;
            auto const color     = color::get<LevelV>();
            auto const timestamp = std::chrono::system_clock::now();
            auto const filename  = source_location.file_name();
            auto const line      = source_location.line();
            std::println("{}{:%Y-%m-%d %H:%M:%S} [{:^8}] {}:{} {}{}", color, timestamp, level, filename, line, message,
                         color::reset);
        }
    };

    template <typename... ArgsT>
    struct trace : public dispatcher<level::trace, ArgsT...> {
        constexpr trace(std::format_string<ArgsT...> const& format, ArgsT&&... args,
                        std::source_location const source_location = std::source_location::current()) noexcept :
            dispatcher<level::trace, ArgsT...>(source_location, format, std::forward<ArgsT>(args)...) {}
    };

    template <typename... ArgsT>
    trace(std::format_string<ArgsT...> const& format, ArgsT&&...) noexcept -> trace<ArgsT...>;

    template <typename... ArgsT>
    struct debug : public dispatcher<level::debug, ArgsT...> {
        constexpr debug(std::format_string<ArgsT...> const& format, ArgsT&&... args,
                        std::source_location const source_location = std::source_location::current()) noexcept :
            dispatcher<level::debug, ArgsT...>(source_location, format, std::forward<ArgsT>(args)...) {}
    };

    template <typename... ArgsT>
    debug(std::format_string<ArgsT...> const& format, ArgsT&&...) noexcept -> debug<ArgsT...>;

    template <typename... ArgsT>
    struct warn : public dispatcher<level::warn, ArgsT...> {
        constexpr warn(std::format_string<ArgsT...> const& format, ArgsT&&... args,
                       std::source_location const source_location = std::source_location::current()) noexcept :
            dispatcher<level::warn, ArgsT...>(source_location, format, std::forward<ArgsT>(args)...) {}
    };

    template <typename... ArgsT>
    warn(std::format_string<ArgsT...> const& format, ArgsT&&...) noexcept -> warn<ArgsT...>;

    template <typename... ArgsT>
    struct info : public dispatcher<level::info, ArgsT...> {
        constexpr info(std::format_string<ArgsT...> const& format, ArgsT&&... args,
                       std::source_location const source_location = std::source_location::current()) noexcept :
            dispatcher<level::info, ArgsT...>(source_location, format, std::forward<ArgsT>(args)...) {}
    };

    template <typename... ArgsT>
    info(std::format_string<ArgsT...> const& format, ArgsT&&...) noexcept -> info<ArgsT...>;

    template <typename... ArgsT>
    struct error : public dispatcher<level::error, ArgsT...> {
        constexpr error(std::format_string<ArgsT...> const& format, ArgsT&&... args,
                        std::source_location const source_location = std::source_location::current()) noexcept :
            dispatcher<level::error, ArgsT...>(source_location, format, std::forward<ArgsT>(args)...) {}
    };

    template <typename... ArgsT>
    error(std::format_string<ArgsT...> const& format, ArgsT&&...) noexcept -> error<ArgsT...>;

    template <typename... ArgsT>
    struct fatal : public dispatcher<level::fatal, ArgsT...> {
        constexpr fatal(std::format_string<ArgsT...> const& format, ArgsT&&... args,
                        std::source_location const source_location = std::source_location::current()) noexcept :
            dispatcher<level::fatal, ArgsT...>(source_location, format, std::forward<ArgsT>(args)...) {}
    };

    template <typename... ArgsT>
    fatal(std::format_string<ArgsT...> const& format, ArgsT&&...) noexcept -> fatal<ArgsT...>;

} // namespace spl::logger

#ifdef NDEBUG
#    define SPL_LOG_TRACE(...)
#    define SPL_LOG_DEBUG(...)
#    define SPL_LOG_INFO(...)  spl::logger::info(__VA_ARGS__)
#    define SPL_LOG_WARN(...)  spl::logger::warn(__VA_ARGS__)
#    define SPL_LOG_ERROR(...) spl::logger::error(__VA_ARGS__)
#    define SPL_LOG_FATAL(...) spl::logger::fatal(__VA_ARGS__)
#else
#    define SPL_LOG_TRACE(...) spl::logger::trace(__VA_ARGS__)
#    define SPL_LOG_DEBUG(...) spl::logger::debug(__VA_ARGS__)
#    define SPL_LOG_INFO(...)  spl::logger::info(__VA_ARGS__)
#    define SPL_LOG_WARN(...)  spl::logger::warn(__VA_ARGS__)
#    define SPL_LOG_ERROR(...) spl::logger::error(__VA_ARGS__)
#    define SPL_LOG_FATAL(...) spl::logger::fatal(__VA_ARGS__)
#endif
