#include "spl/metrics/timeline.hpp"
#include "spl/metrics/stream/max.hpp"
#include "spl/metrics/stream/min.hpp"
#include "spl/metrics/stream/mean.hpp"
#include "spl/metrics/stream/median.hpp"
#include "spl/protocol/feeder/trade/trade_summary.hpp"

#include <gtest/gtest.h>

using namespace spl::protocol;

using timeline_type = spl::metrics::timeline<feeder::trade::trade_summary>;

constexpr auto add(timeline_type& timeline, auto& stream_metric, spl::types::price price, int64_t timestamp) -> void {
    auto const& trade = timeline.emplace_back(feeder::trade::trade_summary{
        .price     = price,
        .timestamp = std::chrono::nanoseconds(timestamp),
    });
    stream_metric(trade);
}

TEST(StreamTest, MaxFindsHighestPrice) {
    auto timeline   = timeline_type{std::chrono::seconds(10)};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, max_stream, 100.5_p, 1'000'000'000);
    add(timeline, max_stream, 150.75_p, 2'000'000'000);
    add(timeline, max_stream, 125.25_p, 3'000'000'000);
    add(timeline, max_stream, 200.00_p, 4'000'000'000); // Maximum
    add(timeline, max_stream, 175.50_p, 5'000'000'000);

    auto result = max_stream();

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 200.0_p);
}

TEST(StreamTest, MinFindsLowestPrice) {
    auto timeline   = timeline_type{std::chrono::seconds(10)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};

    add(timeline, min_stream, 100.5_p, 1'000'000'000);
    add(timeline, min_stream, 150.75_p, 2'000'000'000);
    add(timeline, min_stream, 75.25_p, 3'000'000'000); // Minimum
    add(timeline, min_stream, 200.00_p, 4'000'000'000);
    add(timeline, min_stream, 175.50_p, 5'000'000'000);

    auto result = min_stream();

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 75.25_p);
}

TEST(StreamTest, MeanCalculatesAverage) {
    auto timeline    = timeline_type{std::chrono::seconds(10)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    // Add 5 trades: 100, 150, 125, 200, 175
    // Average = (100 + 150 + 125 + 200 + 175) / 5 = 750 / 5 = 150
    add(timeline, mean_stream, 100.0_p, 1'000'000'000);
    add(timeline, mean_stream, 150.0_p, 2'000'000'000);
    add(timeline, mean_stream, 125.0_p, 3'000'000'000);
    add(timeline, mean_stream, 200.0_p, 4'000'000'000);
    add(timeline, mean_stream, 175.0_p, 5'000'000'000);

    auto result = mean_stream();

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 150.0_p);
}

TEST(StreamTest, MedianWithOddNumberOfElements) {
    auto timeline      = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    // Add 5 trades (odd number): sorted prices will be [100, 125, 150, 175, 200]
    // Median = 150
    add(timeline, median_stream, 100.0_p, 1'000'000'000);
    add(timeline, median_stream, 200.0_p, 2'000'000'000);
    add(timeline, median_stream, 150.0_p, 3'000'000'000);
    add(timeline, median_stream, 125.0_p, 4'000'000'000);
    add(timeline, median_stream, 175.0_p, 5'000'000'000);

    auto result = median_stream();

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 150.0_p);
}

TEST(StreamTest, MedianWithEvenNumberOfElements) {
    auto timeline      = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    // Add 6 trades (even number): sorted prices will be [100, 120, 140, 160, 180, 200]
    // Median = (140 + 160) / 2 = 150
    add(timeline, median_stream, 100.0_p, 1'000'000'000);
    add(timeline, median_stream, 200.0_p, 2'000'000'000);
    add(timeline, median_stream, 140.0_p, 3'000'000'000);
    add(timeline, median_stream, 160.0_p, 4'000'000'000);
    add(timeline, median_stream, 120.0_p, 5'000'000'000);
    add(timeline, median_stream, 180.0_p, 6'000'000'000);

    auto result = median_stream();

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 150.0_p);
}

TEST(StreamTest, SingleElementTimeline) {
    auto timeline      = timeline_type{std::chrono::seconds(10)};
    auto max_stream    = spl::metrics::stream::max<feeder::trade::trade_summary>{};
    auto min_stream    = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto mean_stream   = spl::metrics::stream::mean<feeder::trade::trade_summary>{};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, max_stream, 123.45_p, 1'000'000'000);
    add(timeline, min_stream, 123.45_p, 1'000'000'000);
    add(timeline, mean_stream, 123.45_p, 1'000'000'000);
    add(timeline, median_stream, 123.45_p, 1'000'000'000);

    auto max_result    = max_stream();
    auto min_result    = min_stream();
    auto mean_result   = mean_stream();
    auto median_result = median_stream();

    ASSERT_TRUE(max_result);
    ASSERT_TRUE(min_result);
    ASSERT_TRUE(mean_result);
    ASSERT_TRUE(median_result);

    EXPECT_EQ(max_result.value(), 123.45_p);
    EXPECT_EQ(min_result.value(), 123.45_p);
    EXPECT_EQ(mean_result.value(), 123.45_p);
    EXPECT_EQ(median_result.value(), 123.45_p);
}

TEST(StreamTest, RealisticTradeDataset) {
    auto timeline      = timeline_type{std::chrono::seconds(10)};
    auto max_stream    = spl::metrics::stream::max<feeder::trade::trade_summary>{};
    auto min_stream    = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto mean_stream   = spl::metrics::stream::mean<feeder::trade::trade_summary>{};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    // Simulate a realistic trading scenario with price fluctuations
    std::vector<std::pair<double, int64_t>> trades = {
        {50000.50, 1 }, // Starting price
        {50005.75, 2 }, // Small increase
        {49995.25, 3 }, // Drop
        {50010.00, 4 }, // Recovery
        {50020.50, 5 }, // Increase
        {50015.00, 6 }, // Small drop
        {50025.75, 7 }, // Peak
        {50012.50, 8 }, // Decline
        {50008.25, 9 }, // Further decline
        {50003.00, 10}  // Near start
    };

    for (auto const& [price, time] : trades) {
        auto const& trade =
            timeline.emplace_back(feeder::trade::trade_summary{{},
                                                               {},
                                                               {},
                                                               {},
                                                               spl::types::price::from(price),
                                                               {},
                                                               {},
                                                               {},
                                                               std::chrono::nanoseconds(time * 1'000'000'000)});
        max_stream(trade);
        min_stream(trade);
        mean_stream(trade);
        median_stream(trade);
    }

    auto max_result    = max_stream();
    auto min_result    = min_stream();
    auto mean_result   = mean_stream();
    auto median_result = median_stream();

    ASSERT_TRUE(max_result);
    ASSERT_TRUE(min_result);
    ASSERT_TRUE(mean_result);
    ASSERT_TRUE(median_result);

    // Verify max and min
    EXPECT_EQ(max_result.value(), 50025.75_p);
    EXPECT_EQ(min_result.value(), 49995.25_p);

    // Calculate expected mean
    double sum = 0.0;
    for (auto const& [price, _] : trades) {
        sum += price;
    }
    auto expected_mean = spl::types::price::from(sum / trades.size());
    EXPECT_EQ(mean_result.value(), expected_mean);

    // Median of sorted prices: [49995.25, 50000.50, 50003.00, 50005.75, 50008.25,
    //                            50010.00, 50012.50, 50015.00, 50020.50, 50025.75]
    // Median = (50008.25 + 50010.00) / 2 = 50009.125
    EXPECT_EQ(median_result.value(), 50009.125_p);
}

TEST(StreamTest, DuplicatePricesHandledCorrectly) {
    auto timeline    = timeline_type{std::chrono::seconds(10)};
    auto max_stream  = spl::metrics::stream::max<feeder::trade::trade_summary>{};
    auto min_stream  = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    // Add multiple trades at same price
    add(timeline, max_stream, 100.0_p, 1'000'000'000);
    add(timeline, min_stream, 100.0_p, 1'000'000'000);
    add(timeline, mean_stream, 100.0_p, 1'000'000'000);

    add(timeline, max_stream, 100.0_p, 2'000'000'000);
    add(timeline, min_stream, 100.0_p, 2'000'000'000);
    add(timeline, mean_stream, 100.0_p, 2'000'000'000);

    add(timeline, max_stream, 100.0_p, 3'000'000'000);
    add(timeline, min_stream, 100.0_p, 3'000'000'000);
    add(timeline, mean_stream, 100.0_p, 3'000'000'000);

    EXPECT_EQ(max_stream().value(), 100.0_p);
    EXPECT_EQ(min_stream().value(), 100.0_p);
    EXPECT_EQ(mean_stream().value(), 100.0_p);
}

TEST(StreamTest, SlidingWindowWithRemoval) {
    auto timeline    = timeline_type{std::chrono::seconds(5)};
    auto max_stream  = spl::metrics::stream::max<feeder::trade::trade_summary>{};
    auto min_stream  = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    // Add trades within the 5-second window
    // t=1s: 100
    auto const& trade1 = timeline.emplace_back(
        feeder::trade::trade_summary{.price = 100.0_p, .timestamp = std::chrono::nanoseconds(1'000'000'000)});
    max_stream(trade1);
    min_stream(trade1);
    mean_stream(trade1);

    // t=2s: 200 (will be max)
    auto const& trade2 = timeline.emplace_back(
        feeder::trade::trade_summary{.price = 200.0_p, .timestamp = std::chrono::nanoseconds(2'000'000'000)});
    max_stream(trade2);
    min_stream(trade2);
    mean_stream(trade2);

    // t=3s: 50 (will be min)
    auto const& trade3 = timeline.emplace_back(
        feeder::trade::trade_summary{.price = 50.0_p, .timestamp = std::chrono::nanoseconds(3'000'000'000)});
    max_stream(trade3);
    min_stream(trade3);
    mean_stream(trade3);

    // Verify initial state: 100, 200, 50
    // Mean = (100 + 200 + 50) / 3 = 116.666...
    EXPECT_EQ(max_stream().value(), 200.0_p);
    EXPECT_EQ(min_stream().value(), 50.0_p);
    EXPECT_NEAR(static_cast<double>(mean_stream().value()), 116.66666666, 0.01);

    // Flush old data (elements older than 7 seconds from now = remove t=1s and t=2s)
    auto now = std::chrono::nanoseconds(7'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) {
        max_stream(begin, end);
        min_stream(begin, end);
        mean_stream(begin, end);
    });

    // After flush, only the trade at t=3s (50.0) should remain
    EXPECT_EQ(max_stream().value(), 50.0_p);
    EXPECT_EQ(min_stream().value(), 50.0_p);
    EXPECT_EQ(mean_stream().value(), 50.0_p);
}

TEST(StreamTest, RemovalOfDuplicatePricesAtFront) {
    auto timeline   = timeline_type{std::chrono::seconds(5)};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};

    // Add multiple trades at same price (will be at front of monotonic deque)
    add(timeline, max_stream, 100.0_p, 1'000'000'000);
    add(timeline, min_stream, 100.0_p, 1'000'000'000);

    add(timeline, max_stream, 100.0_p, 2'000'000'000);
    add(timeline, min_stream, 100.0_p, 2'000'000'000);

    add(timeline, max_stream, 100.0_p, 3'000'000'000);
    add(timeline, min_stream, 100.0_p, 3'000'000'000);

    add(timeline, max_stream, 50.0_p, 4'000'000'000);
    add(timeline, min_stream, 150.0_p, 4'000'000'000);

    // max deque should have [100 (count=3), 50 (count=1)]
    // min deque should have [100 (count=3), 150 (count=1)]
    EXPECT_EQ(max_stream().value(), 100.0_p);
    EXPECT_EQ(min_stream().value(), 100.0_p);

    // Flush first trade (t=1s) - should decrement count
    auto now = std::chrono::nanoseconds(6'500'000'000);
    timeline.flush(now, [&](auto begin, auto end) {
        max_stream(begin, end);
        min_stream(begin, end);
    });

    // After removing t=1s, count should be 2, still 100
    EXPECT_EQ(max_stream().value(), 100.0_p);
    EXPECT_EQ(min_stream().value(), 100.0_p);

    // Flush more (t=1s, t=2s gone)
    now = std::chrono::nanoseconds(7'500'000'000);
    timeline.flush(now, [&](auto begin, auto end) {
        max_stream(begin, end);
        min_stream(begin, end);
    });

    // After removing t=1s and t=2s, count should be 1, still 100
    EXPECT_EQ(max_stream().value(), 100.0_p);
    EXPECT_EQ(min_stream().value(), 100.0_p);

    // Flush all 100.0 prices
    now = std::chrono::nanoseconds(8'500'000'000);
    timeline.flush(now, [&](auto begin, auto end) {
        max_stream(begin, end);
        min_stream(begin, end);
    });

    // Now only t=4s remains
    EXPECT_EQ(max_stream().value(), 50.0_p);
    EXPECT_EQ(min_stream().value(), 150.0_p);
}
