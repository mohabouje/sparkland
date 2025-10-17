#pragma once

#include "spl/metrics/timeline.hpp"
#include "spl/result/result.hpp"
#include "spl/types/price.hpp"

namespace spl::metrics::scan {

    template <typename ObjectT,                                     //
              template <typename...> class ContainerT = std::deque, //
              typename PredicateT                     = internal::timeline_predicate>
    struct min {
        using container_type = spl::metrics::timeline<ObjectT, ContainerT, PredicateT>;

        constexpr explicit min(container_type& reference) noexcept : reference_{reference} {}

        [[nodiscard]] constexpr auto operator()() const -> result<spl::types::price> {
            if (std::empty(reference_)) [[unlikely]] {
                return spl::failure("Cannot compute min of an empty timeline");
            }

            auto const iter = std::min_element(std::cbegin(reference_), std::cend(reference_),
                                               [](auto const& lhs, auto const& rhs) { return lhs.price < rhs.price; });
            return iter->price;
        }

    private:
        container_type& reference_;
    };

} // namespace spl::metrics::scan