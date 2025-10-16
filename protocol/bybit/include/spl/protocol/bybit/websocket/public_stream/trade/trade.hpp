#pragma once

#include <spl/reflect/reflect.hpp>

#include <string>

namespace spl::protocol::bybit::websocket::public_stream::trade {

    struct data {
        std::string i;
        std::int64_t T;
        std::string p;
        std::string v;
        std::string S;
        std::int64_t seq;
        std::string s;
        bool BT;
        bool RPI;
        std::optional<std::string> L;
    };

    struct trade {
        std::string topic;
        std::string type;
        std::int64_t ts;
        std::vector<spl::protocol::bybit::websocket::public_stream::trade::data> data;
    };

} // namespace spl::protocol::bybit::websocket::public_stream::trade

namespace spl::reflect {

    template <>
    struct identifier<spl::protocol::bybit::websocket::public_stream::trade::trade> {
        using value_type                = spl::protocol::bybit::websocket::public_stream::trade::trade;
        constexpr static auto unique_id = std::string_view("trade:snapshot");
        constexpr static auto hash      = spl::concepts::xxhash::xxh32(unique_id);
    };

} // namespace spl::reflect
