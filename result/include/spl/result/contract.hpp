#pragma once



#include <string_view>
#include <iostream>
#include <format>

namespace spl { inline namespace outcome {

    template <typename... ArgsT>
    [[gnu::always_inline]] constexpr auto violation([[maybe_unused]] bool expr,
                                                    [[maybe_unused]] std::format_string<ArgsT...> const& msg,
                                                    [[maybe_unused]] ArgsT&&... args) {
#ifdef SPL_CONTRACTS_ENABLED
        if (not expr) [[unlikely]] {
            std::cerr << std::format(msg, std::forward<ArgsT>(args)...) << std::endl;
            std::terminate();
        }
#endif
    }

    template <typename... ArgsT>
    [[gnu::always_inline]] constexpr auto expects(bool condition, std::format_string<ArgsT...> const& msg,
                                                  ArgsT&&... args) {
        violation(condition, msg, std::forward<ArgsT>(args)...);
    }

    template <typename... ArgsT>
    [[gnu::always_inline]] constexpr auto ensures(bool condition, std::format_string<ArgsT...> const& msg,
                                                  ArgsT&&... args) {
        violation(condition, msg, std::forward<ArgsT>(args)...);
    }

}} // namespace spl::outcome
