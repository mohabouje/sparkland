#pragma once

#include <deque>
#include <chrono>
#include <algorithm>
#include <utility>
#include <iterator>

namespace spl::metrics {

    namespace internal {
        struct timeline_predicate {
            template <typename EventT>
            [[nodiscard]] constexpr auto operator()(EventT&& value) const noexcept -> std::chrono::nanoseconds {
                return value.timestamp;
            }
        };
    } // namespace internal

    template <typename T,                                           //
              template <typename...> class ContainerT = std::deque, //
              typename PredicateT                     = internal::timeline_predicate>
    struct timeline {
        using value_type      = T;
        using duration_type   = std::chrono::nanoseconds;
        using container_type  = ContainerT<value_type>;
        using iterator        = typename container_type::iterator;
        using const_iterator  = typename container_type::const_iterator;
        using reference       = typename container_type::reference;
        using const_reference = typename container_type::const_reference;
        using predicate_type  = PredicateT;

        constexpr timeline(duration_type period) noexcept : period_{period} {}

        [[nodiscard]] constexpr auto duration() const noexcept -> duration_type {
            if (std::size(values_) < 2) {
                return std::chrono::nanoseconds::zero();
            }

            auto const& first = values_.front();
            auto const& last  = values_.back();
            auto predicate    = predicate_type{};
            return predicate(last) - predicate(first);
        }

        [[nodiscard]] constexpr auto size() const noexcept -> std::size_t {
            return std::size(values_);
        }

        [[nodiscard]] constexpr auto empty() const noexcept -> bool {
            return std::empty(values_);
        }

        [[nodiscard]] constexpr auto operator[](std::size_t index) noexcept -> reference {
            return values_[index];
        }

        [[nodiscard]] constexpr auto operator[](std::size_t index) const noexcept -> const_reference {
            return values_[index];
        }

        [[nodiscard]] constexpr auto begin() noexcept -> iterator {
            return std::begin(values_);
        }

        [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator {
            return std::begin(values_);
        }

        [[nodiscard]] constexpr auto cbegin() const noexcept -> const_iterator {
            return std::cbegin(values_);
        }

        [[nodiscard]] constexpr auto end() noexcept -> iterator {
            return std::end(values_);
        }

        [[nodiscard]] constexpr auto end() const noexcept -> const_iterator {
            return std:: end(values_);
        }

        [[nodiscard]] constexpr auto cend() const noexcept -> const_iterator {
            return std::cend(values_);
        }

        [[nodiscard]] constexpr auto front() noexcept -> reference {
            return values_.front();
        }

        [[nodiscard]] constexpr auto front() const noexcept -> const_reference {
            return values_.front();
        }

        [[nodiscard]] constexpr auto back() noexcept -> reference {
            return values_.back();
        }

        [[nodiscard]] constexpr auto back() const noexcept -> const_reference {
            return values_.back();
        }

        template <typename... ArgsT>
        [[nodiscard]] constexpr auto emplace_back(ArgsT&&... args) noexcept -> reference {
            return this->emplace_back<true>(std::forward<ArgsT>(args)...);
        }

        template <bool FlushV, typename... ArgsT>
        [[nodiscard]] constexpr auto emplace_back(ArgsT&&... args) noexcept -> reference {
            values_.emplace_back(std::forward<ArgsT>(args)...);
            if constexpr (FlushV) {
                flush();
            }
            return back();
        }

        constexpr auto pop_front() noexcept -> void {
            if constexpr (requires { values_.pop_front(); }) {
                values_.pop_front();
            } else {
                values_.erase(std::begin(values_));
            }
        }

        constexpr auto clear() noexcept -> void {
            values_.clear();
        }

        constexpr auto flush() noexcept -> void {
            if (std::empty(values_)) {
                return;
            }

            if (duration() < period_) [[likely]] {
                return;
            }

            return flush(predicate_type{}(values_.back()));
        }

        constexpr auto flush(duration_type last) noexcept -> void {
            auto const first    = predicate_type{}(values_.front());
            auto const duration = last - first;
            if (duration < period_) [[likely]] {
                return;
            }

            auto const iter = std::find_if(std::cbegin(values_), std::cend(values_),
                                           [&](auto&& value) { return (last - predicate_type{}(value)) < period_; });
            values_.erase(std::begin(values_), iter);
        }

        template <typename HandlerT>
        constexpr auto flush(duration_type last, HandlerT&& handler) noexcept -> void {
            auto const first    = predicate_type{}(values_.front());
            auto const duration = last - first;
            if (duration < period_) [[likely]] {
                return;
            }

            auto const iter = std::find_if(std::begin(values_), std::end(values_),
                                           [&](auto&& value) { return (last - predicate_type{}(value)) < period_; });
            handler(std::begin(values_), iter);
            values_.erase(std::begin(values_), iter);
        }

    private:
        duration_type period_;
        container_type values_{};
    };

} // namespace spl::metrics
