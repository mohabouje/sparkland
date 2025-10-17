#pragma once

#include "spl/protocol/coinbase/websocket/public_stream/subscribe/channel.hpp"

#include <spl/reflect/reflect.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace spl::protocol::coinbase::websocket::public_stream::subscribe {

    struct response {
        std::string type;
        std::vector<channel> channels;
    };

} // namespace spl::protocol::coinbase::websocket::public_stream::subscribe

namespace spl::reflect {

    template <>
    struct identifier<spl::protocol::coinbase::websocket::public_stream::subscribe::response> {
        using value_type                = spl::protocol::coinbase::websocket::public_stream::subscribe::response;
        constexpr static auto unique_id = std::string_view("subscriptions");
        constexpr static auto hash      = spl::concepts::xxhash::xxh32(unique_id);
    };

} // namespace spl::reflect
