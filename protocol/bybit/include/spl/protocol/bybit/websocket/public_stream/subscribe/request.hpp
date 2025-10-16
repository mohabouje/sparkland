#pragma once

#include <spl/reflect/reflect.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace spl::protocol::bybit::websocket::public_stream::subscribe {

    struct request {
        std::string req_id;
        std::vector<std::string> args;
    };

} // namespace spl::protocol::bybit::websocket::public_stream::subscribe

namespace spl::reflect {

    template <>
    struct identifier<spl::protocol::bybit::websocket::public_stream::subscribe::request> {
        using value_type                = spl::protocol::bybit::websocket::public_stream::subscribe::request;
        constexpr static auto unique_id = std::string_view("subscribe");
        constexpr static auto hash      = spl::concepts::xxhash::xxh32(unique_id);
    };

} // namespace spl::reflect
