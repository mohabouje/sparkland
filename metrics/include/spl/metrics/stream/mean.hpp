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
     * - Update (remove): O(1) where k is number of removed elements, amortized O(1)
     *
     */
    template <typename ObjectT,                                     //
              template <typename...> class ContainerT = std::deque, //
              typename PredicateT                     = internal::timeline_predicate>
    struct mean {
        using value_type = spl::types::price;

        constexpr mean() noexcept = default;

        [[nodiscard]] constexpr auto operator()() const noexcept -> spl::types::price {
            return value_type::from(accumulated_ / static_cast<double>(count_));
        }

        template <typename IteratorT>
        constexpr auto operator()(IteratorT begin, IteratorT end) noexcept -> void {
            for (auto it = begin; it != end; ++it) {
                accumulated_ -= static_cast<double>(it->price);
                --count_;
            }
        }

        constexpr auto operator()(ObjectT const& value) noexcept -> void {
            accumulated_ += static_cast<double>(value.price);
            ++count_;
        }

    private:
        double accumulated_{0.0}; ///< Running sum of all prices
        std::size_t count_{0};    ///< Count of elements in window
    };

} // namespace spl::metrics::stream
