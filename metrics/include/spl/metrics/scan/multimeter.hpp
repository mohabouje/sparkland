#pragma once

#include "spl/metrics/timeline.hpp"
#include "spl/metrics/metrics.hpp"
#include "spl/metrics/scan/median.hpp"
#include "spl/metrics/scan/max.hpp"
#include "spl/metrics/scan/min.hpp"
#include "spl/metrics/scan/mean.hpp"

namespace spl::metrics::scan {

    template <typename ObjectT,                                     //
              template <typename...> class ContainerT = std::deque, //
              typename PredicateT                     = internal::timeline_predicate>
    struct multimeter {
        constexpr explicit multimeter(std::chrono::nanoseconds period = std::chrono::milliseconds{100}) noexcept :
            timeline_{period}, median_{timeline_}, max_{timeline_}, min_{timeline_}, mean_{timeline_} {}

        template <typename InstanceT>
        requires std::is_same_v<std::decay_t<InstanceT>, ObjectT>
        [[nodiscard]] constexpr auto operator()(InstanceT&& instance) noexcept -> spl::metrics::metrics {
            auto& reference      = timeline_.emplace_back(std::forward<InstanceT>(instance));
            auto const timestamp = PredicateT{}(reference);
            return spl::metrics::metrics{
                .minimum   = min_(),
                .maximum   = max_(),
                .median    = median_(),
                .mean      = mean_(),
                .timestamp = timestamp,
            };
        }

    private:
        spl::metrics::timeline<ObjectT, ContainerT, PredicateT> timeline_;
        spl::metrics::scan::median<ObjectT, ContainerT, PredicateT> median_;
        spl::metrics::scan::max<ObjectT, ContainerT, PredicateT> max_;
        spl::metrics::scan::min<ObjectT, ContainerT, PredicateT> min_;
        spl::metrics::scan::mean<ObjectT, ContainerT, PredicateT> mean_;
    };

} // namespace spl::metrics::scan
