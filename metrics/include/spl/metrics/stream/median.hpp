#pragma once

#include "spl/metrics/timeline.hpp"
#include "spl/result/result.hpp"
#include "spl/types/price.hpp"
#include "spl/container/flat_unordered_set.hpp"

#include <queue>
#include <vector>
#include <chrono>

namespace spl::metrics::stream {

    /**
     * @brief O(log N) streaming median using dual heaps with lazy deletion
     *
     * Maintains two heaps: max-heap for lower half, min-heap for upper half.
     * Uses lazy deletion to handle element removal efficiently by storing removed
     * objects in an unordered_set and cleaning them up when they reach heap tops.
     *
     * @tparam ObjectT The object type stored in the timeline
     *                 Requirements: .price, .timestamp, .sequence fields
     *                 Must be hashable (std::hash<ObjectT>) and equality comparable
     * @tparam ContainerT The underlying container type for the timeline
     * @tparam PredicateT Predicate to extract timestamp from ObjectT
     *
     * @par Complexity
     * - Query: O(1) - returns median from heap tops
     * - Insert: O(log N) amortized - heap push + rebalance + cleanup
     * - Remove: O(1) average - hash set insertion, O(log N) amortized for cleanup
     *
     */
    template <typename ObjectT,                                     //
              template <typename...> class ContainerT = std::deque, //
              typename PredicateT                     = internal::timeline_predicate>
    struct median {
        using value_type     = spl::types::price;
        using timestamp_type = std::chrono::nanoseconds;

        constexpr median() noexcept = default;

        [[nodiscard]] constexpr auto operator()() const -> result<value_type> {
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

        template <typename IteratorT>
        constexpr auto operator()(IteratorT begin, IteratorT end) noexcept -> void {
            for (auto it = begin; it != end; ++it) {
                removed_objects_.insert(*it);
            }

            cleanup_heaps();
        }

        constexpr auto operator()(ObjectT const& value) noexcept -> void {
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
        constexpr auto cleanup_heaps() noexcept -> void {
            while (!max_heap_.empty()) {
                if (removed_objects_.count(max_heap_.top())) {
                    removed_objects_.erase(max_heap_.top());
                    max_heap_.pop();
                } else {
                    break;
                }
            }

            while (!min_heap_.empty()) {
                if (removed_objects_.count(min_heap_.top())) {
                    removed_objects_.erase(min_heap_.top());
                    min_heap_.pop();
                } else {
                    break;
                }
            }
        }

        /**
         * @brief Maintain heap size balance (sizes differ by at most 1)
         * @complexity O(log N)
         */
        constexpr auto rebalance() noexcept -> void {
            if (max_heap_.size() > min_heap_.size() + 1) {
                min_heap_.push(max_heap_.top());
                max_heap_.pop();
            } else if (min_heap_.size() > max_heap_.size() + 1) {
                max_heap_.push(min_heap_.top());
                min_heap_.pop();
            }
        }

        struct comparator_max {
            [[nodiscard]] constexpr auto operator()(ObjectT const& a, ObjectT const& b) const noexcept -> bool {
                if (a.price == b.price) [[unlikely]] {
                    if (a.timestamp == b.timestamp) [[unlikely]] {
                        return a.sequence > b.sequence;
                    }
                    return a.timestamp > b.timestamp;
                }
                return a.price < b.price;
            }
        };

        struct comparator_min {
            [[nodiscard]] constexpr auto operator()(ObjectT const& a, ObjectT const& b) const noexcept -> bool {
                if (a.price == b.price) [[unlikely]] {
                    if (a.timestamp == b.timestamp) [[unlikely]] {
                        return a.sequence > b.sequence;
                    }
                    return a.timestamp > b.timestamp;
                }
                return a.price > b.price;
            }
        };

        using max_heap_type = std::priority_queue<ObjectT, std::vector<ObjectT>, comparator_max>;
        using min_heap_type = std::priority_queue<ObjectT, std::vector<ObjectT>, comparator_min>;

        max_heap_type max_heap_{};
        min_heap_type min_heap_{};
        spl::container::flat_unordered_set<ObjectT> removed_objects_{};
    };

} // namespace spl::metrics::stream
