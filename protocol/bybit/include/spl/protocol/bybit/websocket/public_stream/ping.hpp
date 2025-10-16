#pragma once

#include <spl/reflect/reflect.hpp>

#include <optional>
#include <string>
#include <string_view>

namespace spl::protocol::bybit::websocket::public_stream {

    struct ping {
        std::optional<std::string> req_id;
        std::optional<std::string> conn_id;
        std::optional<std::string> ret_msg;
    };

} // namespace spl::protocol::bybit::websocket::public_stream

namespace spl::reflect {

    template <>
    struct identifier<spl::protocol::bybit::websocket::public_stream::ping> {
        using value_type                = spl::protocol::bybit::websocket::public_stream::ping;
        constexpr static auto unique_id = std::string_view("ping");
        constexpr static auto hash      = spl::concepts::xxhash::xxh32(unique_id);
    };

} // namespace spl::reflect
