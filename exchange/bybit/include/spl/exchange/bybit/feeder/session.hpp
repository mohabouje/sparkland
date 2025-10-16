#pragma once

#include "spl/exchange/bybit/feeder/contract.hpp"
#include "spl/components/feeder/codegen.hpp"

namespace spl::exchange::bybit::feeder {

    template <spl::exchange::common::environment EnvironmentV>
    using session = spl::components::feeder::codegen<contract<EnvironmentV>>;

} // namespace spl::exchange::bybit::feeder