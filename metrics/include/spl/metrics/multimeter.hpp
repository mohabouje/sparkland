#pragma once

#include "spl/metrics/type.hpp"
#include "spl/metrics/stream/multimeter.hpp"
#include "spl/metrics/scan/multimeter.hpp"
#include "spl/meta/map.hpp"
#include "spl/meta/typed.hpp"

namespace spl::metrics {

    namespace internal {

        template <typename ObjectT,                                     //
                  template <typename...> class ContainerT = std::deque, //
                  typename PredicateT                     = internal::timeline_predicate>
        using multimeter_lookup =
            spl::meta::map<spl::meta::vpair<spl::metrics::type::scan,
                                            spl::metrics::scan::multimeter<ObjectT, ContainerT, PredicateT>>, //
                           spl::meta::vpair<spl::metrics::type::stream,
                                            spl::metrics::stream::multimeter<ObjectT, ContainerT, PredicateT>>>;

    } // namespace internal

    template <spl::metrics::type TypeV,                             //
              typename ObjectT,                                     //
              template <typename...> class ContainerT = std::deque, //
              typename PredicateT                     = internal::timeline_predicate>
    using multimeter = spl::meta::map_at<internal::multimeter_lookup<ObjectT, ContainerT, PredicateT>, //
                                         spl::meta::typed<TypeV>>;

} // namespace spl::metrics
