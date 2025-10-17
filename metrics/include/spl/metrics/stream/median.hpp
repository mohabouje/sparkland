#pragma once

#include "spl/metrics/timeline.hpp"
#include "spl/result/result.hpp"
#include "spl/types/price.hpp"

#include <queue>
#include <vector>
#include <unordered_set>
#include <chrono>

namespace spl::metrics::stream {

    /**
     * @brief O(log N) streaming median using dual heaps with lazy deletion
     *
     * Maintains two heaps: max-heap for lower half, min-heap for upper half.
     * Uses lazy deletion to handle element removal efficiently.
     *
     * @tparam ObjectT The object type stored in the timeline (must have .price and timestamp)
     * @tparam ContainerT The underlying container type for the timeline
     * @tparam PredicateT Predicate to extract timestamp from ObjectT
     *
     * @par Complexity
     * - Query: O(1)
     * - Update (insert): O(log N)
     * - Update (remove): O(k log N) where k is number of removed elements (amortized)
     *
     * @par Example
     * @code
     * spl::metrics::timeline<Trade> timeline{std::chrono::seconds(60)};
     * auto median_metric = spl::metrics::stream::median<Trade>{timeline};
     *
     * // Insert new element
     * timeline.emplace_back(trade);
     * median_metric(trade);
     *
     * // Remove old elements
     * timeline.flush(now, [&](auto begin, auto end) {
     *     median_metric(begin, end);
     * });
     *
     * // Query median
     * auto median_price = median_metric();
     * @endcode
     */
    template <typename ObjectT,                                     //
              template <typename...> class ContainerT = std::deque, //
              typename PredicateT                     = internal::timeline_predicate>
    struct median {
        using value_type     = spl::types::price;
        using timestamp_type = std::chrono::nanoseconds;

        struct timestamp_hash {
            std::size_t operator()(timestamp_type const& t) const noexcept {
                return std::hash<typename timestamp_type::rep>{}(t.count());
            }
        };

        struct comparator_max {
            constexpr auto operator()(ObjectT const& a, ObjectT const& b) const noexcept -> bool {
                return a.price < b.price; // Max-heap: larger prices on top
            }
        };

        struct comparator_min {
            constexpr auto operator()(ObjectT const& a, ObjectT const& b) const noexcept -> bool {
                return a.price > b.price; // Min-heap: smaller prices on top
            }
        };

        using max_heap_type = std::priority_queue<ObjectT, std::vector<ObjectT>, comparator_max>;
        using min_heap_type = std::priority_queue<ObjectT, std::vector<ObjectT>, comparator_min>;

        /**
         * @brief Construct a streaming median metric
         */
        constexpr median() noexcept = default;

        /**
         * @brief Query the current median value
         * @return The median price (or average of two middle values), or error if empty
         * @complexity O(1)
         */
        [[nodiscard]] auto operator()() const -> result<value_type> {
            if (max_heap_.empty() && min_heap_.empty()) [[unlikely]] {
                return spl::failure("Cannot compute median of an empty timeline");
            }

            if (max_heap_.size() > min_heap_.size()) {
                return max_heap_.top().price;
            } else if (max_heap_.size() < min_heap_.size()) {
                return min_heap_.top().price;
            } else {
                // Equal sizes: return average
                auto const lower = max_heap_.top().price;
                auto const upper = min_heap_.top().price;
                return value_type::from((static_cast<double>(lower) + static_cast<double>(upper)) / 2.0);
            }
        }

        /**
         * @brief Update metric when elements are removed from timeline
         * @param begin Iterator to first removed element
         * @param end Iterator past last removed element
         * @complexity O(k log N) where k is number of removed elements (lazy deletion with cleanup)
         */
        template <typename IteratorT>
        auto operator()(IteratorT begin, IteratorT end) noexcept -> void {
            for (auto it = begin; it != end; ++it) {
                auto const timestamp = PredicateT{}(*it);
                removed_timestamps_.insert(timestamp);
            }

            // Lazy deletion: clean up heap tops if they're marked for removal
            cleanup_heaps();
        }

        /**
         * @brief Update metric when a new element is inserted
         * @param value The newly inserted object
         * @complexity O(log N) - heap insertion and rebalancing
         */
        auto operator()(ObjectT const& value) noexcept -> void {
            if (max_heap_.empty() || value.price <= max_heap_.top().price) {
                max_heap_.push(value);
            } else {
                min_heap_.push(value);
            }

            rebalance();
            cleanup_heaps();
        }

    private:
        /**
         * @brief Remove lazy-deleted elements from heap tops
         * @complexity O(log N) per removed element
         */
        auto cleanup_heaps() noexcept -> void {
            while (!max_heap_.empty()) {
                auto const timestamp = PredicateT{}(max_heap_.top());
                if (removed_timestamps_.count(timestamp)) {
                    max_heap_.pop();
                    removed_timestamps_.erase(timestamp);
                } else {
                    break;
                }
            }

            while (!min_heap_.empty()) {
                auto const timestamp = PredicateT{}(min_heap_.top());
                if (removed_timestamps_.count(timestamp)) {
                    min_heap_.pop();
                    removed_timestamps_.erase(timestamp);
                } else {
                    break;
                }
            }
        }

        /**
         * @brief Maintain heap size balance (sizes differ by at most 1)
         * @complexity O(log N)
         */
        auto rebalance() noexcept -> void {
            if (max_heap_.size() > min_heap_.size() + 1) {
                min_heap_.push(max_heap_.top());
                max_heap_.pop();
            } else if (min_heap_.size() > max_heap_.size() + 1) {
                max_heap_.push(min_heap_.top());
                min_heap_.pop();
            }
        }

        mutable max_heap_type max_heap_{};                                                ///< Lower half (max on top)
        mutable min_heap_type min_heap_{};                                                ///< Upper half (min on top)
        mutable std::unordered_multiset<timestamp_type, timestamp_hash> removed_timestamps_{}; ///< Lazy deletion tracking
    };

} // namespace spl::metrics::stream
