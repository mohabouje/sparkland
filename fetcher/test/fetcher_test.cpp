#include "spl/fetcher/fetcher.hpp"

#include <gtest/gtest.h>

TEST(FetcherTest, BasicFetchingTimeBasedCandleStickFromBybit) {
    constexpr auto exchange_id = spl::protocol::common::exchange_id::bybit;
    constexpr auto type        = spl::candlestick::type::time;
    using fetcher_type         = spl::fetcher::fetcher<exchange_id, type>;

    auto context    = spl::network::context{};
    auto session_id = spl::components::feeder::session_id{"fetcher", "bybit"};
    auto fetcher    = fetcher_type{context, session_id, std::chrono::seconds(1)};

    auto const connected = fetcher.connect();
    ASSERT_TRUE(connected) << "Failed to connect: " << connected.error().message().data();

    auto const subscribed = fetcher(spl::protocol::feeder::stream::subscribe{
        .exchange_id   = spl::protocol::common::exchange_id::bybit,
        .instrument_id = spl::protocol::common::instrument_id{"BTCUSDT"},
        .channel       = spl::protocol::feeder::stream::channel::trades,
    });
    ASSERT_TRUE(subscribed) << "Failed to send subscription request: " << subscribed.error().message().data();

    auto const now      = std::chrono::steady_clock::now();
    auto const deadline = now + std::chrono::seconds(10);
    while (std::chrono::steady_clock::now() < deadline) {
        auto const operation = fetcher.poll([]<typename EventT>(EventT&& event) -> spl::result<void> {
            spl::logger::info("  -> {}", std::forward<EventT>(event));
            return spl::success();
        });
        ASSERT_TRUE(operation) << "Failed to poll events: " << operation.error().message().data();
    }
}