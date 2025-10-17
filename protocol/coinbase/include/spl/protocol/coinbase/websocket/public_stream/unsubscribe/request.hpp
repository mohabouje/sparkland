#pragma once

#include "spl/protocol/coinbase/websocket/public_stream/subscribe/channel.hpp"

#include <spl/reflect/reflect.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace spl::protocol::coinbase::websocket::public_stream::unsubscribe {

    struct request {
        std::string type;
        std::vector<spl::protocol::coinbase::websocket::public_stream::subscribe::channel> channels;
    };

} // namespace spl::protocol::coinbase::websocket::public_stream::unsubscribe

namespace spl::reflect {

    template <>
    struct identifier<spl::protocol::coinbase::websocket::public_stream::unsubscribe::request> {
        using value_type                = spl::protocol::coinbase::websocket::public_stream::unsubscribe::request;
        constexpr static auto unique_id = std::string_view("unsubscribe");
        constexpr static auto hash      = spl::concepts::xxhash::xxh32(unique_id);
    };

} // namespace spl::reflect
