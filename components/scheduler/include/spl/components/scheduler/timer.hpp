#pragma once

#include "spl/result/result.hpp"
#include "spl/components/scheduler/timer_id.hpp"

#include <optional>
#include <chrono>

namespace spl::components::scheduler {

    enum class timer_type { periodic, oneshot };

    template <timer_type Type>
    struct timer_t;

    template <>
    struct timer_t<timer_type::periodic> {
        using time_point = std::chrono::steady_clock::time_point;
        using duration   = std::chrono::steady_clock::duration;
        using function   = std::function<spl::result<void>()>;

        template <typename FunctorT>
        constexpr timer_t(timer_id id, duration period, FunctorT&& callback) noexcept :
            id_(id), period_(period), callback_(std::forward<FunctorT>(callback)) {}

        [[nodiscard]] constexpr auto id() const noexcept -> timer_id {
            return id_;
        }

        [[nodiscard]] constexpr auto trigger() const noexcept -> time_point const& {
            return trigger_.value();
        }

        [[nodiscard]] constexpr auto period() const noexcept -> duration {
            return period_;
        }

        [[nodiscard, gnu::hot]] constexpr auto try_trigger(time_point now) noexcept -> spl::result<bool> {
            if (not trigger_ or (now < trigger_)) [[unlikely]] {
                return false;
            }

            trigger_ = now + period_;
            err_return(callback_());
            return true;
        }

        [[nodiscard]] constexpr auto started() const noexcept -> bool {
            return trigger_.has_value();
        }

        constexpr auto start(time_point now) noexcept -> void {
            trigger_ = now + period_;
        }

        constexpr auto stop() noexcept -> void {
            trigger_ = std::nullopt;
        }

        constexpr auto reset() noexcept -> void {
            trigger_ = std::nullopt;
        }

    private:
        timer_id id_;
        std::optional<time_point> trigger_;
        duration period_;
        function callback_;
    };

    template <>
    struct timer_t<timer_type::oneshot> {
        using time_point = std::chrono::steady_clock::time_point;
        using duration   = std::chrono::steady_clock::duration;
        using function   = std::function<spl::result<void>()>;

        template <typename FunctorT>
        constexpr timer_t(timer_id id, duration period, FunctorT&& callback) noexcept :
            id_(id), period_(period), callback_(std::forward<FunctorT>(callback)) {}

        [[nodiscard]] constexpr auto id() const noexcept -> timer_id {
            return id_;
        }

        [[nodiscard]] constexpr auto period() const noexcept -> duration {
            return period_;
        }

        [[nodiscard]] constexpr auto trigger() const noexcept -> time_point const& {
            return trigger_.value();
        }

        [[nodiscard]] constexpr auto expired(time_point now) const noexcept -> bool {
            return trigger_ and now >= trigger_;
        }

        [[nodiscard, gnu::hot]] constexpr auto try_trigger(time_point now) noexcept -> spl::result<bool> {
            if (not trigger_ or (now < trigger_)) [[unlikely]] {
                return false;
            }

            trigger_ = std::nullopt;
            err_return(callback_());
            return true;
        }

        [[nodiscard]] constexpr auto started() const noexcept -> bool {
            return trigger_.has_value();
        }

        constexpr auto start(time_point now) noexcept -> void {
            trigger_ = now + period_;
        }

        constexpr auto stop() noexcept -> void {
            trigger_ = std::nullopt;
        }

        constexpr auto reset() noexcept -> void {
            trigger_ = std::nullopt;
        }

    private:
        timer_id id_;
        std::optional<time_point> trigger_;
        duration period_;
        function callback_;
    };

    using timer = std::variant<timer_t<timer_type::periodic>, timer_t<timer_type::oneshot>>;

} // namespace spl::components::scheduler