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
        template <typename InstanceT>
        requires std::is_same_v<std::decay_t<InstanceT>, ObjectT>
        [[nodiscard]] constexpr auto operator()(InstanceT&& instance) noexcept -> spl::metrics::metrics {
            std::ignore          = timeline_.emplace_back(std::forward<InstanceT>(instance));
            auto const median    = median_(timeline_);
            auto const maximum   = max_(timeline_);
            auto const minimum   = min_(timeline_);
            auto const mean      = mean_(timeline_);
            auto const timestamp = PredicateT{}(instance);
            return spl::metrics::metrics{
                .minimum   = minimum.value(),
                .maximum   = maximum.value(),
                .median    = median.value(),
                .mean      = mean.value(),
                .timestamp = timestamp,
            };
        }

    private:
        spl::metrics::timeline timeline_;
        spl::metrics::scan::median median_;
        spl::metrics::scan::max max_;
        spl::metrics::scan::min min_;
        spl::metrics::scan::mean mean_;
    };

} // namespace spl::metrics::scan
