#pragma once

#include "spl/network/socket/stream.hpp"

#include <boost/asio/ip/tcp.hpp>

namespace spl::network::client {

    using tcp = socket::stream<boost::asio::ip::tcp>;

}