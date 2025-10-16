#pragma once

#include "spl/components/scheduler/timer_id.hpp"
#include "spl/components/scheduler/timer.hpp"
#include "spl/result/result.hpp"
#include "spl/result/contract.hpp"
#include "spl/core/unused.hpp"

namespace spl::components::scheduler {

    struct scheduler {
        using key_type   = spl::components::scheduler::timer_id;
        using value_type = spl::components::scheduler::timer;
        using duration   = std::chrono::steady_clock::duration;
        using time_point = std::chrono::steady_clock::time_point;

        template <typename FunctorT>
        [[nodiscard]] constexpr auto periodic(duration period, FunctorT&& functor) -> timer_t<timer_type::periodic>& {
            ensures(not full(), "timer service is full, cannot add more timers: {}/{}", size(), capacity());
            using timer_type = timer_t<timer_type::periodic>;
            auto& ref        = storage_.emplace_back(timer_type(unique_id_++, period, std::forward<FunctorT>(functor)));
            return std::get<timer_type>(ref);
        }

        template <typename FunctorT>
        [[nodiscard]] constexpr auto periodic(time_point now, duration period, FunctorT&& functor) -> timer_id {
            ensures(not full(), "timer service is full, cannot add more timers: {}/{}", size(), capacity());
            using timer_type = timer_t<timer_type::periodic>;
            auto& ref        = storage_.emplace_back(timer_type(unique_id_++, period, std::forward<FunctorT>(functor)));
            auto& und        = std::get<timer_type>(ref);
            und.start(now);
            return und.id();
        }

        template <typename ObjectT, typename MethodT>
        [[nodiscard]] constexpr auto periodic(duration period, ObjectT&& object, MethodT method)
            -> timer_t<timer_type::periodic>& {
            auto const functor = [obj = std::forward<ObjectT>(object), method]() { return std::invoke(method, obj); };
            return periodic(period, functor);
        }

        template <typename ObjectT, typename MethodT>
        [[nodiscard]] constexpr auto periodic(time_point now, duration period, ObjectT&& object, MethodT method)
            -> timer_id {
            auto const functor = [obj = std::forward<ObjectT>(object), method]() { return std::invoke(method, obj); };
            return periodic(now, period, functor);
        }

        template <typename FunctorT>
        [[nodiscard]] constexpr auto oneshot(duration period, FunctorT&& functor) -> timer_t<timer_type::oneshot>& {
            ensures(not full(), "timer service is full, cannot add more timers: {}/{}", size(), capacity());
            using timer_type = timer_t<timer_type::oneshot>;
            auto& ref        = storage_.emplace_back(timer_type(unique_id_++, period, std::forward<FunctorT>(functor)));
            return std::get<timer_type>(ref);
        }

        template <typename FunctorT>
        [[nodiscard]] constexpr auto oneshot(time_point now, duration period, FunctorT&& functor) -> timer_id {
            ensures(not full(), "timer service is full, cannot add more timers: {}/{}", size(), capacity());
            using timer_type = timer_t<timer_type::oneshot>;
            auto& ref        = storage_.emplace_back(timer_type(unique_id_++, period, std::forward<FunctorT>(functor)));
            auto& und        = std::get<timer_type>(ref);
            und.start(now);
            return und.id();
        }

        template <typename ObjectT, typename MethodT>
        [[nodiscard]] constexpr auto oneshot(duration period, ObjectT&& object, MethodT method)
            -> timer_t<timer_type::oneshot>& {
            auto const functor = [obj = std::forward<ObjectT>(object), method]() { return std::invoke(method, obj); };
            return oneshot(period, functor);
        }

        template <typename ObjectT, typename MethodT>
        [[nodiscard]] constexpr auto oneshot(time_point now, duration period, ObjectT&& object, MethodT method)
            -> timer_id {
            auto const functor = [obj = std::forward<ObjectT>(object), method]() { return std::invoke(method, obj); };
            return oneshot(now, period, functor);
        }

        template <typename FunctorT>
        [[nodiscard]] constexpr auto submit(FunctorT&& functor) -> timer_id {
            ensures(not full(), "timer service is full, cannot add more timers: {}/{}", size(), capacity());
            using timer_type = timer_t<timer_type::oneshot>;
            auto& ref        = storage_.emplace_back(timer_type(unique_id_++, {0}, std::forward<FunctorT>(functor)));
            auto& und        = std::get<timer_type>(ref);
            und.start(std::chrono::steady_clock::now());
            return und.id();
        }

        [[nodiscard]] constexpr auto contains(timer_id id) const noexcept -> bool {
            return std::any_of(std::begin(storage_), std::end(storage_), [id](auto&& value) {
                auto const current_id = std::visit([](auto&& instance) { return instance.id(); }, value);
                return current_id == id;
            });
        }

        [[nodiscard]] constexpr auto size() const noexcept -> std::size_t {
            return std::size(storage_);
        }

        [[nodiscard]] constexpr auto capacity() const noexcept -> std::size_t {
            return 512;
        }

        [[nodiscard]] constexpr auto full() const noexcept -> bool {
            return std::size(storage_) == capacity();
        }

        template <typename HandlerT>
        [[nodiscard]] constexpr auto poll(HandlerT&& handler) noexcept -> result<void> {
            unused(handler, "this service does not produce any events");
            auto triggered = false;
            for (auto const now = std::chrono::steady_clock::now(); auto& value : storage_) {
                auto const functor = [&now](auto&& instance) -> result<bool> { return instance.try_trigger(now); };
                triggered |= err_return(std::visit(functor, value));
            }

            if (not triggered) [[likely]] {
                return spl::success();
            }

            auto const iter = std::remove_if(std::begin(storage_), std::end(storage_), [](auto&& value) {
                return std::visit([](auto&& instance) { return not instance.started(); }, value);
            });
            std::ignore     = storage_.erase(iter, std::end(storage_));
            return spl::success();
        }

    private:
        components::scheduler::timer_id unique_id_{0};
        std::vector<value_type> storage_;
    };

} // namespace spl::components::scheduler
