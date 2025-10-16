#pragma once

#include "spl/protocol/common/exchange_id.hpp"
#include "spl/protocol/common/instrument_id.hpp"
#include "spl/protocol/common/timestamp.hpp"
#include "spl/protocol/connection/connection_id.hpp"
#include "spl/protocol/connection/status.hpp"

namespace spl::protocol::feeder::stream {

    struct connection {
        spl::protocol::common::exchange_id exchange_id;
        spl::protocol::common::instrument_id instrument_id;
        spl::protocol::connection::connection_id connection_id;
        spl::protocol::connection::status status;
        spl::protocol::common::timestamp timestamp;
    };

} // namespace spl::protocol::feeder::stream
