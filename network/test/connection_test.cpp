#include "spl/logger/logger.hpp"
#include "spl/network/connector/router.hpp"
#include "spl/network/client/tls.hpp"
#include "spl/network/client/wss.hpp"
#include "spl/network/client/udp.hpp"
#include "spl/network/common/uri_parser.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

TEST(NetworkTest, EstablishConnectionWebSocket) {
    auto context = spl::network::context{};
    auto router  = spl::network::connector::router<spl::network::client::wss>{context};

    auto const url  = std::string("wss://stream.bybit.com/v5/public/spot");
    auto const port = std::string("443");
    auto const host = spl::network::common::host(url).value();
    auto const path = spl::network::common::path(url).value();
    auto connection = router.make_connection(host, port, path);
    ASSERT_TRUE(connection) << connection.error().message().data();
}

TEST(NetworkTest, ProcessConnectionWebSocket_Blocking) {
    auto context = spl::network::context{};
    auto router  = spl::network::connector::router<spl::network::client::wss>{context};

    auto const url      = std::string("wss://ws.bitstamp.net/");
    auto const port     = std::string("443");
    auto const host     = spl::network::common::host(url).value();
    auto const path     = spl::network::common::path(url).value();
    auto make_operation = router.make_connection(host, port, path);
    ASSERT_TRUE(make_operation) << make_operation.error().message().data();

    auto const data = std::string(R"(
        {
            "event": "bts:subscribe",
            "data": {
                "channel" : "trades"
            }
        }
    )");

    auto& connection           = make_operation.value();
    auto const write_operation = connection.write(boost::asio::buffer(data));
    ASSERT_EQ(write_operation, std::size(data));

    std::this_thread::sleep_for(std::chrono::seconds{1});

    ASSERT_GT(connection.bytes_readable().value(), 0);

    boost::beast::flat_buffer buffer;
    std::ignore = connection.read(buffer);
    std::cout << "Received: " << boost::beast::make_printable(buffer.data()) << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds{1});

    ASSERT_EQ(connection.bytes_readable().value(), 0);
}

TEST(NetworkTest, EstablishConnectionFIXGateway) {
    auto context = spl::network::context{};
    auto router  = spl::network::connector::router<spl::network::client::tls>{context};

    auto const url    = std::string("tcp+ssl://fix-public.sandbox.exchange.coinbase.com/");
    auto const port   = std::string("4198");
    auto const host   = spl::network::common::host(url).value();
    auto const path   = spl::network::common::path(url).value();
    auto connection   = std::optional<spl::network::client::tls>{};
    auto const result = router.make_connection(connection, host, port, path);
    ASSERT_TRUE(result) << result.error().message().data();
}