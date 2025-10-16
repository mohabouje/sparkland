#pragma once

#include <spl/reflect/reflect.hpp>

#include <string>
#include <string_view>

namespace spl::protocol::bybit::websocket::public_stream::unsubscribe {

    struct response {
        std::string req_id;
        std::string conn_id;
        std::string ret_msg;
        bool success;
    };

} // namespace spl::protocol::bybit::websocket::public_stream::unsubscribe

namespace spl::reflect {

    template <>
    struct identifier<spl::protocol::bybit::websocket::public_stream::unsubscribe::response> {
        using value_type                = spl::protocol::bybit::websocket::public_stream::unsubscribe::response;
        constexpr static auto unique_id = std::string_view("unsubscribe");
        constexpr static auto hash      = spl::concepts::xxhash::xxh32(unique_id);
    };

} // namespace spl::reflect
