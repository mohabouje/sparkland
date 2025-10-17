#pragma once

#include <spl/reflect/reflect.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace spl::protocol::coinbase::websocket::public_stream::ticker {

    struct ticker {
        std::int64_t sequence;
        std::string product_id;
        std::string price;
        std::string open_24h;
        std::string volume_24h;
        std::string low_24h;
        std::string high_24h;
        std::string volume_30d;
        std::string best_bid;
        std::string best_bid_size;
        std::string best_ask;
        std::string best_ask_size;
        std::string side;
        std::string time;
        std::int64_t trade_id;
        std::string last_size;
    };

} // namespace spl::protocol::coinbase::websocket::public_stream::ticker

namespace spl::reflect {

    template <>
    struct identifier<spl::protocol::coinbase::websocket::public_stream::ticker::ticker> {
        using value_type                = spl::protocol::coinbase::websocket::public_stream::ticker::ticker;
        constexpr static auto unique_id = std::string_view("ticker");
        constexpr static auto hash      = spl::concepts::xxhash::xxh32(unique_id);
    };

} // namespace spl::reflect
