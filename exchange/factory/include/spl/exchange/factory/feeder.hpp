#pragma once

#include "spl/components/feeder/codegen.hpp"
#include "spl/protocol/common/exchange_id.hpp"
#include "spl/exchange/bybit/feeder/contract.hpp"
#include "spl/exchange/coinbase/feeder/contract.hpp"
#include "spl/meta/map.hpp"
#include "spl/meta/list.hpp"

namespace spl::exchange::factory {

    namespace internal {

        template <spl::exchange::common::environment EnvironmentV>
        using lookup = spl::meta::map< //
            spl::meta::vpair<spl::protocol::common::exchange_id::bybit,
                             spl::exchange::bybit::feeder::contract<EnvironmentV>>,
            spl::meta::vpair<spl::protocol::common::exchange_id::coinbase,
                             spl::exchange::coinbase::feeder::contract<EnvironmentV>>>;

        template <spl::protocol::common::exchange_id ExchangeIdV, //
                  spl::exchange::common::environment EnvironmentV = spl::exchange::common::environment::production>
        using contract = spl::meta::map_at<spl::exchange::factory::internal::lookup<EnvironmentV>, //
                                           spl::meta::typed<ExchangeIdV>>;

    } // namespace internal

    template <spl::protocol::common::exchange_id ExchangeIdV, //
              spl::exchange::common::environment EnvironmentV = spl::exchange::common::environment::production>
    using feeder = spl::components::feeder::codegen<internal::contract<ExchangeIdV, EnvironmentV>>;

} // namespace spl::exchange::factory
