#pragma once

#include <spl/reflect/reflect.hpp>

#include <string_view>

namespace spl::protocol::bybit::websocket::public_stream {

    struct pong {};

} // namespace spl::protocol::bybit::websocket::public_stream

namespace spl::reflect {

    template <>
    struct identifier<spl::protocol::bybit::websocket::public_stream::pong> {
        using value_type                = spl::protocol::bybit::websocket::public_stream::pong;
        constexpr static auto unique_id = std::string_view("pong");
        constexpr static auto hash      = spl::concepts::xxhash::xxh32(unique_id);
    };

} // namespace spl::reflect
