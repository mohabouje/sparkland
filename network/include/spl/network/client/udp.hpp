#pragma once

#include "spl/network/socket/datagram.hpp"

#include <boost/asio/ip/udp.hpp>

namespace spl::network::client {

    using udp = socket::datagram<boost::asio::ip::udp>;

}