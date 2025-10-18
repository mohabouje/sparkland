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
        constexpr explicit multimeter(std::chrono::nanoseconds period = std::chrono::milliseconds{100}) noexcept :
            timeline_{period} {}

        template <typename InstanceT>
        requires std::is_same_v<std::decay_t<InstanceT>, ObjectT>
        [[nodiscard]] constexpr auto operator()(InstanceT&& instance) noexcept -> spl::metrics::metrics {
            auto& reference      = timeline_.emplace_back(std::forward<InstanceT>(instance));
            auto const timestamp = PredicateT{}(reference);
            timeline_.flush(timestamp, [this](auto first, auto last) {
                median_(first, last);
                max_(first, last);
                min_(first, last);
                mean_(first, last);
            });
            emit(reference, median_, max_, min_, mean_);
            return spl::metrics::metrics{
                .minimum   = min_(),
                .maximum   = max_(),
                .median    = median_(),
                .mean      = mean_(),
                .timestamp = timestamp,
            };
        }

    private:
        template <typename InstanceT, typename... SubscribersT>
        constexpr auto emit(InstanceT&& instance, SubscribersT&&... subscribers) noexcept -> void {
            (std::forward<SubscribersT>(subscribers)(instance), ...);
        }

        spl::metrics::timeline<ObjectT, ContainerT, PredicateT> timeline_;
        spl::metrics::stream::median<ObjectT, ContainerT, PredicateT> median_;
        spl::metrics::stream::max<ObjectT, ContainerT, PredicateT> max_;
        spl::metrics::stream::min<ObjectT, ContainerT, PredicateT> min_;
        spl::metrics::stream::mean<ObjectT, ContainerT, PredicateT> mean_;
    };

} // namespace spl::metrics::stream
