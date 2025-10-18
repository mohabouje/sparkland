#pragma once

#include "spl/metrics/timeline.hpp"
#include "spl/result/result.hpp"
#include "spl/types/price.hpp"

#include <vector>
#include <algorithm>

namespace spl::metrics::scan {

    template <typename ObjectT,                                     //
              template <typename...> class ContainerT = std::deque, //
              typename PredicateT                     = internal::timeline_predicate>
    struct median {
        using container_type = spl::metrics::timeline<ObjectT, ContainerT, PredicateT>;

        constexpr explicit median(container_type& reference) noexcept : reference_{reference} {}

        [[nodiscard]] constexpr auto operator()() const noexcept -> result<spl::types::price> {
            auto tmp = std::vector<spl::types::price>{std::size(reference_)};
            std::transform(std::cbegin(reference_), std::cend(reference_), std::begin(tmp),
                           [](auto const& value) { return value.price; });
            auto const n   = std::size(tmp);
            auto const k   = n / 2;
            auto const mid = std::begin(tmp) + k;
            std::nth_element(std::begin(tmp), mid, std::end(tmp));

            if (n & 1) {
                return *mid;
            }

            auto const lower = *std::max_element(std::begin(tmp), mid);
            return spl::types::price((lower + *mid) * 0.5);
        }

    private:
        container_type& reference_;
    };

} // namespace spl::metrics::scan