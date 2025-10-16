#pragma once

#include "spl/protocol/common/exchange_id.hpp"
#include "spl/protocol/common/instrument_id.hpp"
#include "spl/protocol/common/timestamp.hpp"
#include "spl/protocol/feeder/stream/channel.hpp"
#include "spl/protocol/feeder/stream/status.hpp"

namespace spl::protocol::feeder::stream {

    struct subscribed {
        spl::protocol::common::exchange_id exchange_id;
        spl::protocol::common::instrument_id instrument_id;
        spl::protocol::feeder::stream::channel channel;
        spl::protocol::feeder::stream::status status;
        spl::protocol::common::timestamp timestamp;
    };

} // namespace spl::protocol::feeder::stream
