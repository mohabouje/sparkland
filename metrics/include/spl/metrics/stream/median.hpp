#pragma once

#include "spl/metrics/timeline.hpp"
#include "spl/result/result.hpp"
#include "spl/types/price.hpp"

#include <set>
#include <chrono>

namespace spl::metrics::stream {

    /**
     * @brief Streaming median using a single multiset
     *
     * Stores (price, timestamp) pairs to uniquely identify each element.
     * Simple implementation that walks to the middle element(s) on each query.
     *
     * @tparam ObjectT The object type stored in the timeline (must have .price and timestamp)
     * @tparam ContainerT The underlying container type for the timeline
     * @tparam PredicateT Predicate to extract timestamp from ObjectT
     *
     * @par Complexity
     * - Query: O(N) - walks to middle element
     * - Update (insert): O(log N)
     * - Update (remove): O(log N)
     *
     */
    template <typename ObjectT,                                     //
              template <typename...> class ContainerT = std::deque, //
              typename PredicateT                     = internal::timeline_predicate>
    struct median {
        using value_type     = spl::types::price;
        using timestamp_type = std::chrono::nanoseconds;

        struct element {
            value_type price;
            timestamp_type timestamp;
        };

        constexpr median() noexcept = default;

        [[nodiscard]] constexpr auto operator()() const -> result<value_type> {
            auto const size = elements_.size();
            auto const it   = std::next(std::cbegin(elements_), size / 2);
            if (size % 2 == 1) {
                return it->price;
            }

            auto const upper = it->price;
            auto const lower = std::prev(it)->price;
            return value_type::from((static_cast<double>(lower) + static_cast<double>(upper)) / 2.0);
        }

        template <typename IteratorT>
        constexpr auto operator()(IteratorT begin, IteratorT end) noexcept -> void {
            for (auto it = begin; it != end; ++it) {
                auto const timestamp = PredicateT{}(*it);
                element elem{it->price, timestamp};
                elements_.erase(elem);
            }
        }

        constexpr auto operator()(ObjectT const& value) noexcept -> void {
            auto const timestamp = PredicateT{}(value);
            element elem{value.price, timestamp};
            elements_.insert(elem);
        }

    private:
        struct comparator {
            constexpr auto operator()(element const& a, element const& b) const noexcept -> bool {
                if (a.price != b.price) {
                    return a.price < b.price;
                }
                return a.timestamp < b.timestamp;
            }
        };
        using set_type = std::multiset<element, comparator>;
        set_type elements_{};
    };

} // namespace spl::metrics::stream
