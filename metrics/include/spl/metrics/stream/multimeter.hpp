#pragma once

#include "spl/metrics/timeline.hpp"
#include "spl/metrics/metrics.hpp"
#include "spl/metrics/stream/median.hpp"
#include "spl/metrics/stream/max.hpp"
#include "spl/metrics/stream/min.hpp"
#include "spl/metrics/stream/mean.hpp"

namespace spl::metrics::stream {

    template <typename ObjectT,                                     //
              template <typename...> class ContainerT = std::deque, //
              typename PredicateT                     = internal::timeline_predicate>
    struct multimeter {
        template <typename InstanceT>
        requires std::is_same_v<std::decay_t<InstanceT>, ObjectT>
        [[nodiscard]] constexpr auto operator()(InstanceT&& instance) noexcept -> spl::metrics::metrics {
            auto& reference      = timeline_.emplace_back(std::forward<InstanceT>(instance));
            auto const timestamp = PredicateT{}(reference);
            timeline_.flush(timestamp, [](auto first, auto last) {
                median_(first, last);
                max_(first, last);
                min_(first, last);
                mean_(first, last);
            });
            median_(reference);
            max_(reference);
            min_(reference);
            mean_(reference);
            return spl::metrics::metrics{
                .minimum   = min_(),
                .maximum   = max_(),
                .median    = median_(),
                .mean      = mean_(),
                .timestamp = timestamp,
            };
        }

    private:
        spl::metrics::timeline timeline_;
        spl::metrics::stream::median<ObjectT, ContainerT, PredicateT> median_;
        spl::metrics::stream::max<ObjectT, ContainerT, PredicateT> max_;
        spl::metrics::stream::min<ObjectT, ContainerT, PredicateT> min_;
        spl::metrics::stream::mean<ObjectT, ContainerT, PredicateT> mean_;
    };

} // namespace spl::metrics::stream
