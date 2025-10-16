#pragma once

#include "spl/protocol/common/aggressor_side.hpp"
#include "spl/protocol/common/exchange_id.hpp"
#include "spl/protocol/common/instrument_id.hpp"
#include "spl/protocol/common/price.hpp"
#include "spl/protocol/common/quantity.hpp"
#include "spl/protocol/common/sequence.hpp"
#include "spl/protocol/common/timestamp.hpp"
#include "spl/protocol/common/trade_condition.hpp"
#include "spl/protocol/common/trade_id.hpp"

namespace spl::protocol::feeder::trade {

    struct trade_summary {
        spl::protocol::common::instrument_id instrument_id;
        spl::protocol::common::exchange_id exchange_id;
        spl::protocol::common::trade_id trade_id;
        spl::protocol::common::aggressor_side side;
        spl::protocol::common::price price;
        spl::protocol::common::quantity quantity;
        spl::protocol::common::trade_condition condition;
        spl::protocol::common::sequence sequence;
        spl::protocol::common::timestamp timestamp;
    };

} // namespace spl::protocol::feeder::trade
