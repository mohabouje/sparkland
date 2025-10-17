#pragma once

#include "spl/exchange/coinbase/feeder/contract.hpp"
#include "spl/components/feeder/codegen.hpp"

namespace spl::exchange::coinbase::feeder {

    template <spl::exchange::common::environment EnvironmentV>
    using session = spl::components::feeder::codegen<contract<EnvironmentV>>;

} // namespace spl::exchange::coinbase::feeder
