#pragma once

#include "spl/metrics/timeline.hpp"
#include "spl/result/result.hpp"
#include "spl/types/price.hpp"
#include <numeric>

namespace spl::metrics::scan {

    template <typename ObjectT,                                     //
              template <typename...> class ContainerT = std::deque, //
              typename PredicateT                     = internal::timeline_predicate>
    struct mean {
        using container_type = spl::metrics::timeline<ObjectT, ContainerT, PredicateT>;

        constexpr explicit mean(container_type& reference) noexcept : reference_{reference} {}

        [[nodiscard]] constexpr auto operator()() const noexcept -> spl::types::price {
            auto const accumulated =
                std::accumulate(std::cbegin(reference_), std::cend(reference_), spl::types::price{},
                                [](auto const& lhs, auto const& rhs) { return lhs + rhs.price; });
            return accumulated / static_cast<double>(std::size(reference_));
        }

    private:
        container_type& reference_;
    };

} // namespace spl::metrics::scan