#pragma once

#include "spl/meta/always_false.hpp"
#include "spl/result/error_message.hpp"
#include "spl/result/generic_error.hpp"
#include "spl/core/assert.hpp"

#include <boost/outcome/experimental/status_result.hpp>
#include <boost/outcome/experimental/status_outcome.hpp>
#include <boost/outcome/try.hpp>

#include <functional>
#include <type_traits>
#include <utility>
#include <cassert>

namespace spl { inline namespace outcome {

    template <class EC, class E = void>
    using failure_type = boost::outcome_v2::failure_type<EC, E>;

    template <typename T>
    using success_type = boost::outcome_v2::success_type<T>;

    template <typename T, typename ErrorT = generic_error>
    using result =
        boost::outcome_v2::experimental::status_result<T, ErrorT, boost::outcome_v2::experimental::policy::terminate>;

    template <typename T, typename ErrorT = generic_error>
    using result_ref =
        boost::outcome_v2::experimental::status_result<std::reference_wrapper<T>, ErrorT,
                                                       boost::outcome_v2::experimental::policy::terminate>;

    template <typename T, typename ErrorT = generic_error>
    using result_cref =
        boost::outcome_v2::experimental::status_result<std::reference_wrapper<T const>, ErrorT,
                                                       boost::outcome_v2::experimental::policy::terminate>;

    template <typename FirstT, typename... ArgsT>
    requires(not std::is_convertible_v<std::decay_t<FirstT>, std::format_string<FirstT, ArgsT...>>)
    [[nodiscard, gnu::cold]] constexpr auto failure(FirstT&& first, ArgsT&&... args) noexcept -> auto {
        return boost::outcome_v2::failure(std::forward<FirstT>(first), std::forward<ArgsT>(args)...);
    }

    template <typename ErrorT, typename... ArgsT>
    [[nodiscard, gnu::cold]] constexpr auto failure(ArgsT&&... args) noexcept -> failure_type<ErrorT> {
        return boost::outcome_v2::failure(ErrorT{std::forward<ArgsT>(args)...});
    }

    template <typename... ArgsT>
    [[nodiscard, gnu::cold]] constexpr auto failure(std::format_string<ArgsT...> const& format,
                                                    ArgsT&&... args) noexcept -> failure_type<error_message> {
        return boost::outcome_v2::failure(error_message{format, std::forward<ArgsT>(args)...});
    }

    template <typename... ArgsT>
    [[nodiscard, gnu::hot]] constexpr auto success(ArgsT&&... args) noexcept -> auto {
        return boost::outcome_v2::success(std::forward<ArgsT>(args)...);
    }

    template <typename ValueT, typename... ArgsT>
    [[nodiscard, gnu::hot]] constexpr auto success(ArgsT&&... args) noexcept -> success_type<ValueT> {
        return boost::outcome_v2::success(ValueT{std::forward<ArgsT>(args)...});
    }

    template <typename SomethingT>
    [[nodiscard]] constexpr auto failed(SomethingT&& something) noexcept -> bool {
        if constexpr (requires { something.has_error(); }) {
            return something.has_error();
        } else if constexpr (requires { something.has_value(); }) {
            return not something.has_value();
        } else {
            static_assert(spl::always_false<SomethingT>(), "message type cannot be checked for failure");
        }
        std::unreachable();
    }

    template <typename SomethingT>
    [[nodiscard]] constexpr auto succeeded(SomethingT&& something) noexcept -> bool {
        if constexpr (requires { something.has_value(); }) {
            return something.has_value();
        } else if constexpr (requires { something.has_error(); }) {
            return not something.has_error();
        } else {
            static_assert(spl::always_false<SomethingT>(), "message type cannot be checked for success");
        }
        std::unreachable();
    }

    template <typename FunctorT, typename... ArgsT>
    [[nodiscard, gnu::always_inline]] constexpr auto safe_call(FunctorT&& functor, ArgsT&&... args) noexcept
        -> result<std::invoke_result_t<FunctorT, ArgsT...>> {
        try {
            if constexpr (std::is_void_v<std::invoke_result_t<FunctorT, ArgsT...>>) {
                std::invoke(std::forward<FunctorT>(functor), std::forward<ArgsT>(args)...);
                return success();
            } else {
                return success(std::invoke(std::forward<FunctorT>(functor), std::forward<ArgsT>(args)...));
            }
        } catch (std::exception const& exception) {
            return spl::failure<error_message>(exception);
        }
    }

    template <typename SomethingT>
    [[nodiscard, gnu::always_inline]] constexpr auto propagate(SomethingT&& something) noexcept {
        if constexpr (requires { std::forward<SomethingT>(something).has_error(); }) {
            SPL_ASSERT_MSG(something.has_error(), "cannot propagate success");
            return std::forward<SomethingT>(something).error();
        } else {
            static_assert(spl::always_false<SomethingT>(), "message type cannot be propagated");
        }
    }

}} // namespace spl::outcome

#define err_return(...) BOOST_OUTCOME_TRYX(__VA_ARGS__)
