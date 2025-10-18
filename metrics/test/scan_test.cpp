#include "spl/metrics/timeline.hpp"
#include "spl/metrics/scan/max.hpp"
#include "spl/metrics/scan/min.hpp"
#include "spl/metrics/scan/mean.hpp"
#include "spl/metrics/scan/median.hpp"
#include "spl/protocol/feeder/trade/trade_summary.hpp"

#include <gtest/gtest.h>

using namespace spl::protocol;

using timeline_type = spl::metrics::timeline<feeder::trade::trade_summary>;

constexpr auto add(timeline_type& timeline, spl::types::price price, int64_t timestamp) -> void {
    std::ignore = timeline.emplace_back(feeder::trade::trade_summary{
        .price     = price,
        .timestamp = std::chrono::nanoseconds(timestamp),
    });
}

TEST(ScanTest, MaxFindsHighestPrice) {
    auto timeline = timeline_type{std::chrono::seconds(10)};

    add(timeline, 100.5_p, 1'000'000'000);
    add(timeline, 150.75_p, 2'000'000'000);
    add(timeline, 125.25_p, 3'000'000'000);
    add(timeline, 200.00_p, 4'000'000'000); // Maximum
    add(timeline, 175.50_p, 5'000'000'000);

    spl::metrics::scan::max<feeder::trade::trade_summary> max_scan(timeline);
    auto result = max_scan();

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 200.0_p);
}

TEST(ScanTest, MinFindsLowestPrice) {
    auto timeline = timeline_type{std::chrono::seconds(10)};

    add(timeline, 100.5_p, 1'000'000'000);
    add(timeline, 150.75_p, 2'000'000'000);
    add(timeline, 75.25_p, 3'000'000'000); // Minimum
    add(timeline, 200.00_p, 4'000'000'000);
    add(timeline, 175.50_p, 5'000'000'000);

    spl::metrics::scan::min<feeder::trade::trade_summary> min_scan(timeline);
    auto result = min_scan();

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 75.25_p);
}

TEST(ScanTest, MeanCalculatesAverage) {
    auto timeline = timeline_type{std::chrono::seconds(10)};

    // Add 5 trades: 100, 150, 125, 200, 175
    // Average = (100 + 150 + 125 + 200 + 175) / 5 = 750 / 5 = 150
    add(timeline, 100.0_p, 1'000'000'000);
    add(timeline, 150.0_p, 2'000'000'000);
    add(timeline, 125.0_p, 3'000'000'000);
    add(timeline, 200.0_p, 4'000'000'000);
    add(timeline, 175.0_p, 5'000'000'000);

    spl::metrics::scan::mean<feeder::trade::trade_summary> mean_scan(timeline);
    auto result = mean_scan();

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 150.0_p);
}

TEST(ScanTest, MedianWithOddNumberOfElements) {
    auto timeline = timeline_type{std::chrono::seconds(10)};

    // Add 5 trades (odd number): sorted prices will be [100, 125, 150, 175, 200]
    // Median = 150
    add(timeline, 100.0_p, 1'000'000'000);
    add(timeline, 200.0_p, 2'000'000'000);
    add(timeline, 150.0_p, 3'000'000'000);
    add(timeline, 125.0_p, 4'000'000'000);
    add(timeline, 175.0_p, 5'000'000'000);

    spl::metrics::scan::median<feeder::trade::trade_summary> median_scan(timeline);
    auto result = median_scan();

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 150.0_p);
}

TEST(ScanTest, MedianWithEvenNumberOfElements) {
    auto timeline = timeline_type{std::chrono::seconds(10)};

    // Add 6 trades (even number): sorted prices will be [100, 120, 140, 160, 180, 200]
    // Median = (140 + 160) / 2 = 150
    add(timeline, 100.0_p, 1'000'000'000);
    add(timeline, 200.0_p, 2'000'000'000);
    add(timeline, 140.0_p, 3'000'000'000);
    add(timeline, 160.0_p, 4'000'000'000);
    add(timeline, 120.0_p, 5'000'000'000);
    add(timeline, 180.0_p, 6'000'000'000);

    spl::metrics::scan::median<feeder::trade::trade_summary> median_scan(timeline);
    auto result = median_scan();

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 150.0_p);
}

TEST(ScanTest, SingleElementTimeline) {
    auto timeline = timeline_type{std::chrono::seconds(10)};

    add(timeline, 123.45_p, 1'000'000'000);

    spl::metrics::scan::max<feeder::trade::trade_summary> max_scan(timeline);
    spl::metrics::scan::min<feeder::trade::trade_summary> min_scan(timeline);
    spl::metrics::scan::mean<feeder::trade::trade_summary> mean_scan(timeline);
    spl::metrics::scan::median<feeder::trade::trade_summary> median_scan(timeline);

    auto max_result    = max_scan();
    auto min_result    = min_scan();
    auto mean_result   = mean_scan();
    auto median_result = median_scan();

    ASSERT_TRUE(max_result);
    ASSERT_TRUE(min_result);
    ASSERT_TRUE(mean_result);
    ASSERT_TRUE(median_result);

    EXPECT_EQ(max_result.value(), 123.45_p);
    EXPECT_EQ(min_result.value(), 123.45_p);
    EXPECT_EQ(mean_result.value(), 123.45_p);
    EXPECT_EQ(median_result.value(), 123.45_p);
}

TEST(ScanTest, RealisticTradeDataset) {
    auto timeline = timeline_type{std::chrono::seconds(10)};

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
        [[maybe_unused]] auto&& _ =
            timeline.emplace_back(feeder::trade::trade_summary{{},
                                                               {},
                                                               {},
                                                               {},
                                                               spl::types::price::from(price),
                                                               {},
                                                               {},
                                                               {},
                                                               std::chrono::nanoseconds(time * 1'000'000'000)});
    }

    spl::metrics::scan::max<feeder::trade::trade_summary> max_scan(timeline);
    spl::metrics::scan::min<feeder::trade::trade_summary> min_scan(timeline);
    spl::metrics::scan::mean<feeder::trade::trade_summary> mean_scan(timeline);
    spl::metrics::scan::median<feeder::trade::trade_summary> median_scan(timeline);

    auto max_result    = max_scan();
    auto min_result    = min_scan();
    auto mean_result   = mean_scan();
    auto median_result = median_scan();

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

TEST(ScanTest, TimelinePreservesOriginalOrder) {
    auto timeline = timeline_type{std::chrono::seconds(10)};

    add(timeline, 100.0_p, 1'000'000'000);
    add(timeline, 200.0_p, 2'000'000'000);
    add(timeline, 150.0_p, 3'000'000'000);

    // Get median (which internally sorts)
    spl::metrics::scan::median<feeder::trade::trade_summary> median_scan(timeline);
    auto median_result = median_scan();

    ASSERT_TRUE(median_result);

    // Verify timeline order is unchanged
    EXPECT_EQ(timeline[0].price, 100.0_p);
    EXPECT_EQ(timeline[1].price, 200.0_p);
    EXPECT_EQ(timeline[2].price, 150.0_p);
}
