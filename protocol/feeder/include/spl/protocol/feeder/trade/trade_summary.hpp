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

        constexpr auto operator<=>(trade_summary const& other) const noexcept = default;
    };

} // namespace spl::protocol::feeder::trade

template <>
struct std::hash<spl::protocol::feeder::trade::trade_summary> {
    constexpr auto operator()(spl::protocol::feeder::trade::trade_summary const& ts) const noexcept -> std::size_t {
        auto const h1 = std::hash<spl::protocol::common::instrument_id>{}(ts.instrument_id);
        auto const h2 = std::hash<spl::protocol::common::exchange_id>{}(ts.exchange_id);
        auto const h3 = std::hash<spl::protocol::common::trade_id>{}(ts.trade_id);
        auto const h4 = std::hash<spl::protocol::common::price>{}(ts.price);
        auto const h5 = std::hash<spl::protocol::common::quantity>{}(ts.quantity);
        auto const h6 = std::hash<spl::protocol::common::trade_condition>{}(ts.condition);
        auto const h7 = ts.sequence;
        auto const h8 = ts.timestamp.count();
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4) ^ (h6 << 5) ^ (h7 << 6) ^ (h8 << 7);
    }
};