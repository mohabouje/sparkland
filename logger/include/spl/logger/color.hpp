#pragma once

#include "spl/logger/level.hpp"

#include <string_view>

namespace spl::logger {

    struct color {
        constexpr static auto reset = "\033[0m";
        constexpr static auto bold  = "\033[1m";
        constexpr static auto dim   = "\033[2m";

        // Text colors
        constexpr static auto black   = "\033[30m";
        constexpr static auto red     = "\033[31m";
        constexpr static auto green   = "\033[32m";
        constexpr static auto yellow  = "\033[33m";
        constexpr static auto blue    = "\033[34m";
        constexpr static auto magenta = "\033[35m";
        constexpr static auto cyan    = "\033[36m";
        constexpr static auto white   = "\033[37m";

        // Bright colors
        constexpr static auto bright_black   = "\033[90m";
        constexpr static auto bright_red     = "\033[91m";
        constexpr static auto bright_green   = "\033[92m";
        constexpr static auto bright_yellow  = "\033[93m";
        constexpr static auto bright_blue    = "\033[94m";
        constexpr static auto bright_magenta = "\033[95m";
        constexpr static auto bright_cyan    = "\033[96m";
        constexpr static auto bright_white   = "\033[97m";

        [[nodiscard]] constexpr static auto get(logger::level level) noexcept -> std::string_view {
            switch (level) {
                case logger::level::trace:
                    return color::dim;
                case logger::level::debug:
                    return color::cyan;
                case logger::level::info:
                    return color::green;
                case logger::level::warn:
                    return color::yellow;
                case logger::level::error:
                    return color::red;
                case logger::level::fatal:
                    return color::bright_red;
                case logger::level::off:
                    return color::reset;
            }
            std::unreachable();
        }

        template <logger::level LevelV>
        [[nodiscard]] constexpr static auto get() noexcept -> std::string_view {
            if constexpr (LevelV == logger::level::trace) {
                return color::dim;
            } else if constexpr (LevelV == logger::level::debug) {
                return color::cyan;
            } else if constexpr (LevelV == logger::level::info) {
                return color::green;
            } else if constexpr (LevelV == logger::level::warn) {
                return color::yellow;
            } else if constexpr (LevelV == logger::level::error) {
                return color::red;
            } else if constexpr (LevelV == logger::level::fatal) {
                return color::bright_red;
            } else {
                return color::reset;
            }
            return color::reset;
        }
    };

} // namespace spl::logger
