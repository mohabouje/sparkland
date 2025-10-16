#pragma once

#include "spl/protocol/common/exchange_id.hpp"
#include "spl/protocol/common/instrument_id.hpp"
#include "spl/protocol/common/timestamp.hpp"

namespace spl::protocol::feeder::stream {

    struct heartbeat {
        spl::protocol::common::exchange_id exchange_id;
        spl::protocol::common::instrument_id instrument_id;
        spl::protocol::common::timestamp timestamp;
    };

} // namespace spl::protocol::feeder::stream
