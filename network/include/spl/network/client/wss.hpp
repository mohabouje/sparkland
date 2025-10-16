#pragma once

#include "spl/network/socket/layer/websocket.hpp"
#include "spl/network/client/tcp.hpp"
#include "spl/network/client/tls.hpp"

namespace spl::network::client {

    using ws  = socket::layer::websocket<client::tcp>;
    using wss = socket::layer::websocket<client::tls>;

} // namespace spl::network::client