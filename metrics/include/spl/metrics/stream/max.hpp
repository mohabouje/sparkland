#pragma once

#include "spl/metrics/timeline.hpp"
#include "spl/result/result.hpp"
#include "spl/types/price.hpp"

#include <deque>

namespace spl::metrics::stream {

    /**
     * @brief O(1) amortized streaming maximum using monotonic deque with reference counting
     *
     * Maintains a deque of candidate maximums in decreasing price order.
     * Each entry tracks a price and the count of trades at that price.
     * Uses the sliding window maximum algorithm for ultra-low latency queries.
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
     * @par Example
     * @code
     * spl::metrics::timeline<Trade> timeline{std::chrono::seconds(60)};
     * auto max_metric = spl::metrics::stream::max<Trade>{timeline};
     *
     * // Insert new element
     * timeline.emplace_back(trade);
     * max_metric(trade);
     *
     * // Remove old elements
     * timeline.flush(now, [&](auto begin, auto end) {
     *     max_metric(begin, end);
     * });
     *
     * // Query maximum
     * auto max_price = max_metric();
     * @endcode
     */
    template <typename ObjectT,                                     //
              template <typename...> class ContainerT = std::deque, //
              typename PredicateT                     = internal::timeline_predicate>
    struct max {
        using value_type = spl::types::price;

        struct entry {
            value_type price;
            std::size_t count;
        };

        /**
         * @brief Construct a streaming maximum metric
         */
        constexpr max() noexcept = default;

        /**
         * @brief Query the current maximum value
         * @return The maximum price in the current window, or error if empty
         * @complexity O(1)
         */
        [[nodiscard]] constexpr auto operator()() const -> result<value_type> {
            if (std::empty(monotonic_deque_)) [[unlikely]] {
                return spl::failure("Cannot compute max of an empty timeline");
            }
            return monotonic_deque_.front().price;
        }

        /**
         * @brief Update metric when elements are removed from timeline
         * @param begin Iterator to first removed element
         * @param end Iterator past last removed element
         * @complexity O(k) amortized, where k is number of removed elements
         */
        template <typename IteratorT>
        constexpr auto operator()(IteratorT begin, IteratorT end) noexcept -> void {
            // Sliding window: elements leave from the front (oldest first)
            // Only decrement count if the removed price matches the current maximum
            for (auto it = begin; it != end; ++it) {
                if (std::empty(monotonic_deque_)) [[unlikely]] {
                    break;
                }

                auto const price = it->price;

                // Only touch front if this removal affects current maximum
                if (monotonic_deque_.front().price == price) {
                    --monotonic_deque_.front().count;

                    // Clean up zero-count entries from front
                    while (!std::empty(monotonic_deque_) && monotonic_deque_.front().count == 0) {
                        monotonic_deque_.pop_front();
                    }
                }
            }
        }

        /**
         * @brief Update metric when a new element is inserted
         * @param value The newly inserted object
         * @complexity O(1) amortized - maintains monotonic decreasing property
         */
        constexpr auto operator()(ObjectT const& value) noexcept -> void {
            auto const price = value.price;

            // Check if same price already exists at back
            if (!std::empty(monotonic_deque_) && monotonic_deque_.back().price == price) {
                ++monotonic_deque_.back().count;
                return;
            }

            // Remove elements from back that are <= new price (they can't be future maximums)
            while (!std::empty(monotonic_deque_) && monotonic_deque_.back().price <= price) {
                monotonic_deque_.pop_back();
            }

            monotonic_deque_.push_back({price, 1});
        }

    private:
        std::deque<entry> monotonic_deque_{}; ///< Monotonic deque of {price, count} in decreasing price order
    };

} // namespace spl::metrics::stream
