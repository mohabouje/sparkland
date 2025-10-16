#pragma once

#include "spl/network/socket/layer/ssl.hpp"
#include "spl/network/client/tcp.hpp"

namespace spl::network::client {

    using tls = socket::layer::ssl<client::tcp>;

}