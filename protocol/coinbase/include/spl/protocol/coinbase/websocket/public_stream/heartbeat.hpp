#pragma once

#include <spl/reflect/reflect.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace spl::protocol::coinbase::websocket::public_stream {

    struct heartbeat {
        std::string type;
        std::int64_t sequence;
        std::int64_t last_trade_id;
        std::string product_id;
        std::string time;
    };

} // namespace spl::protocol::coinbase::websocket::public_stream

namespace spl::reflect {

    template <>
    struct identifier<spl::protocol::coinbase::websocket::public_stream::heartbeat> {
        using value_type                = spl::protocol::coinbase::websocket::public_stream::heartbeat;
        constexpr static auto unique_id = std::string_view("heartbeat");
        constexpr static auto hash      = spl::concepts::xxhash::xxh32(unique_id);
    };

} // namespace spl::reflect
