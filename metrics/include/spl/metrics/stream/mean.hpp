#pragma once

#include "spl/metrics/timeline.hpp"
#include "spl/result/result.hpp"
#include "spl/types/price.hpp"

namespace spl::metrics::stream {

    /**
     * @brief O(1) streaming arithmetic mean (average price)
     *
     * Maintains running sum and count for constant-time mean calculation.
     * Formula: mean = sum(prices) / count
     *
     * @tparam ObjectT The object type stored in the timeline (must have .price)
     * @tparam ContainerT The underlying container type for the timeline
     * @tparam PredicateT Predicate to extract timestamp from ObjectT
     *
     * @par Complexity
     * - Query: O(1)
     * - Update (insert): O(1)
     * - Update (remove): O(k) where k is number of removed elements
     *
     * @par Example
     * @code
     * spl::metrics::timeline<Trade> timeline{std::chrono::seconds(60)};
     * auto mean_metric = spl::metrics::stream::mean<Trade>{timeline};
     *
     * // Insert new element
     * timeline.emplace_back(trade);
     * mean_metric(trade);
     *
     * // Remove old elements
     * timeline.flush(now, [&](auto begin, auto end) {
     *     mean_metric(begin, end);
     * });
     *
     * // Query mean
     * auto avg_price = mean_metric();
     * @endcode
     */
    template <typename ObjectT,                                     //
              template <typename...> class ContainerT = std::deque, //
              typename PredicateT                     = internal::timeline_predicate>
    struct mean {
        using container_type = spl::metrics::timeline<ObjectT, ContainerT, PredicateT>;
        using value_type     = spl::types::price;

        /**
         * @brief Construct a streaming mean metric
         * @param reference Reference to the timeline container
         */
        constexpr explicit mean(container_type& reference) noexcept : reference_{reference} {}

        /**
         * @brief Query the current mean value
         * @return The arithmetic mean of all prices, or error if empty
         * @complexity O(1)
         */
        [[nodiscard]] constexpr auto operator()() const -> result<value_type> {
            if (count_ == 0) [[unlikely]] {
                return spl::failure("Cannot compute mean of an empty timeline");
            }
            return value_type{accumulated_.value / static_cast<double>(count_)};
        }

        /**
         * @brief Update metric when elements are removed from timeline
         * @param begin Iterator to first removed element
         * @param end Iterator past last removed element
         * @complexity O(k) where k is number of removed elements
         */
        template <typename IteratorT>
        constexpr auto operator()(IteratorT begin, IteratorT end) noexcept -> void {
            for (auto it = begin; it != end; ++it) {
                accumulated_ -= it->price;
                --count_;
            }
        }

        /**
         * @brief Update metric when a new element is inserted
         * @param value The newly inserted object
         * @complexity O(1)
         */
        constexpr auto operator()(ObjectT const& value) noexcept -> void {
            accumulated_ += value.price;
            ++count_;
        }

    private:
        container_type& reference_;
        value_type accumulated_{}; ///< Running sum of all prices
        std::size_t count_{0};     ///< Count of elements in window
    };

} // namespace spl::metrics::stream
