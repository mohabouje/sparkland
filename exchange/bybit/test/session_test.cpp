#include "spl/exchange/bybit/feeder/session.hpp"
#include "spl/exchange/bybit/feeder/transformer.hpp"

#include <gtest/gtest.h>

using production = spl::exchange::bybit::feeder::session<spl::exchange::common::environment::production>;

TEST(ExchangeBybitFeederTest, EstablishConnection) {
    auto context    = spl::network::context();
    auto identifier = spl::components::feeder::session_id{"client", "bybit"};
    auto session    = production(context, identifier);
    auto operation  = session.connect();
    ASSERT_TRUE(operation) << "Failed to connect: " << operation.error().message().data();
}

TEST(ExchangeBybitFeederTest, FetchTradesData) {
    auto context    = spl::network::context();
    auto identifier = spl::components::feeder::session_id{"client", "bybit"};
    auto session    = production(context, identifier);
    auto operation  = session.connect();
    ASSERT_TRUE(operation) << "Failed to connect: " << operation.error().message().data();

    auto const sent = session.send(spl::protocol::feeder::stream::subscribe{
        .exchange_id   = spl::protocol::common::exchange_id::bybit,
        .instrument_id = spl::protocol::common::instrument_id{"BTCUSDT"},
        .channel       = spl::protocol::feeder::stream::channel::trades,
    });
    ASSERT_TRUE(sent) << "Failed to send subscription request: " << sent.error().message().data();

    auto const now      = std::chrono::steady_clock::now();
    auto const deadline = now + std::chrono::seconds(10);
    while (std::chrono::steady_clock::now() < deadline) {
        auto const operation = session.poll([]<typename EventT>(EventT&& event) -> spl::result<void> {
            spl::logger::info("  -> {}", std::forward<EventT>(event));
            return spl::success();
        });
        ASSERT_TRUE(operation) << "Failed to poll events: " << operation.error().message().data();
    }
}
