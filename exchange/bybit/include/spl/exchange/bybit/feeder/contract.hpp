#pragma once

#include "spl/exchange/bybit/feeder/connector.hpp"
#include "spl/exchange/bybit/feeder/tagger.hpp"
#include "spl/exchange/bybit/feeder/transformer.hpp"
#include "spl/components/feeder/session.hpp"
#include "spl/network/client/wss.hpp"
#include "spl/codec/json/decoder.hpp"
#include "spl/codec/json/encoder.hpp"
#include "spl/codec/json/encoder.hpp"
#include "spl/codec/json/encoder.hpp"
#include "spl/protocol/bybit/websocket/public_stream/decoder.hpp"
#include "spl/protocol/bybit/websocket/public_stream/encoder.hpp"

namespace spl::exchange::bybit::feeder {

    template <spl::exchange::common::environment EnvironmentV>
    struct contract {
        using decoder_type = spl::protocol::bybit::websocket::public_stream::decoder<spl::codec::json::decoder, tagger>;
        using encoder_type = spl::protocol::bybit::websocket::public_stream::encoder<spl::codec::json::encoder, tagger>;
        using transformer_type            = spl::exchange::bybit::feeder::transformer;
        using connection_type             = spl::network::client::wss;
        using connector_type              = spl::exchange::bybit::feeder::connector<EnvironmentV>;
        constexpr static auto environment = EnvironmentV;
        constexpr static auto exchange    = spl::protocol::common::exchange_id::bybit;
        constexpr static auto channel     = spl::protocol::feeder::stream::channel::level2;
    };

} // namespace spl::exchange::bybit::feeder
