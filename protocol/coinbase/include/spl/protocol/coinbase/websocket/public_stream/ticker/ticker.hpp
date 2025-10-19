#pragma once

#include <spl/reflect/reflect.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace spl::protocol::coinbase::websocket::public_stream::ticker {

    struct ticker {
        std::int64_t sequence;
        std::string_view product_id;
        std::string_view price;
        std::string_view open_24h;
        std::string_view volume_24h;
        std::string_view low_24h;
        std::string_view high_24h;
        std::string_view volume_30d;
        std::string_view best_bid;
        std::string_view best_bid_size;
        std::string_view best_ask;
        std::string_view best_ask_size;
        std::string_view side;
        std::string_view time;
        std::int64_t trade_id;
        std::string_view last_size;
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
