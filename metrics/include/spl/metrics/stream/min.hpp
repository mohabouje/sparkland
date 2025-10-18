#pragma once

#include "spl/metrics/timeline.hpp"
#include "spl/result/result.hpp"
#include "spl/types/price.hpp"

#include <deque>

namespace spl::metrics::stream {

    /**
     * @brief O(1) amortized streaming minimum using monotonic deque with reference counting
     *
     * Maintains a deque of candidate minimums in increasing price order.
     * Each entry tracks a price and the count of trades at that price.
     * Uses the sliding window minimum algorithm for ultra-low latency queries.
     *
     * @tparam ObjectT The object type stored in the timeline (must have .price)
     * @tparam ContainerT The underlying container type for the timeline
     * @tparam PredicateT Predicate to extract timestamp from ObjectT
     *
     * @par Complexity
     * - Query: O(1)
     * - Update (insert): O(1) amortized
     * - Update (remove): O(k) where k is number of removed elements, amortized O(1)
     *
     */
    template <typename ObjectT,                                     //
              template <typename...> class ContainerT = std::deque, //
              typename PredicateT                     = internal::timeline_predicate>
    struct min {
        using value_type = spl::types::price;

        struct entry {
            value_type price;
            std::size_t count;
        };

        constexpr min() noexcept = default;

        [[nodiscard]] constexpr auto operator()() const noexcept -> result<value_type> {
            return monotonic_deque_.front().price;
        }

        template <typename IteratorT>
        constexpr auto operator()(IteratorT begin, IteratorT end) noexcept -> void {
            for (auto it = begin; it != end; ++it) {
                if (std::empty(monotonic_deque_)) [[unlikely]] {
                    return;
                }

                auto const price = it->price;
                auto& front      = monotonic_deque_.front();
                if (front.price == price) {
                    --front.count;
                    clean();
                }
            }
        }

        constexpr auto operator()(ObjectT const& value) noexcept -> void {
            auto const price = value.price;
            if (not std::empty(monotonic_deque_) and monotonic_deque_.back().price == price) {
                ++monotonic_deque_.back().count;
                return;
            }

            while (not std::empty(monotonic_deque_) and monotonic_deque_.back().price >= price) {
                monotonic_deque_.pop_back();
            }
            monotonic_deque_.push_back({price, 1});
        }

    private:
        constexpr auto clean() noexcept -> void {
            while (not std::empty(monotonic_deque_) and monotonic_deque_.front().count == 0) {
                monotonic_deque_.pop_front();
            }
        }

        std::deque<entry> monotonic_deque_{}; ///< Monotonic deque of {price, count} in increasing price order
    };

} // namespace spl::metrics::stream
