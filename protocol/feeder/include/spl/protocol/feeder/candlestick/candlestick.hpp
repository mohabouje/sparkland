#pragma once

#include "spl/protocol/common/counter.hpp"
#include "spl/protocol/common/exchange_id.hpp"
#include "spl/protocol/common/instrument_id.hpp"
#include "spl/protocol/common/price.hpp"
#include "spl/protocol/common/quantity.hpp"
#include "spl/protocol/common/sequence.hpp"
#include "spl/protocol/common/timestamp.hpp"

namespace spl::protocol::feeder::candlestick {

    struct candlestick {
        spl::protocol::common::instrument_id instrument_id;
        spl::protocol::common::exchange_id exchange_id;
        spl::protocol::common::price open;
        spl::protocol::common::price high;
        spl::protocol::common::price low;
        spl::protocol::common::price close;
        spl::protocol::common::quantity volume;
        spl::protocol::common::counter trades;
        bool complete;
        spl::protocol::common::timestamp starting;
        spl::protocol::common::timestamp ending;
        spl::protocol::common::sequence sequence;
        spl::protocol::common::timestamp timestamp;
    };

} // namespace spl::protocol::feeder::candlestick
