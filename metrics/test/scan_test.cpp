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
    auto const result = max_scan();
    EXPECT_EQ(result, 200.0_p);
}

TEST(ScanTest, MinFindsLowestPrice) {
    auto timeline = timeline_type{std::chrono::seconds(10)};

    add(timeline, 100.5_p, 1'000'000'000);
    add(timeline, 150.75_p, 2'000'000'000);
    add(timeline, 75.25_p, 3'000'000'000); // Minimum
    add(timeline, 200.00_p, 4'000'000'000);
    add(timeline, 175.50_p, 5'000'000'000);

    spl::metrics::scan::min<feeder::trade::trade_summary> min_scan(timeline);
    auto const result = min_scan();
    EXPECT_EQ(result, 75.25_p);
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
    auto const result = mean_scan();
    EXPECT_EQ(result, 150.0_p);
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
    auto const result = median_scan();
    EXPECT_EQ(result, 150.0_p);
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
    auto const result = median_scan();
    EXPECT_EQ(result, 150.0_p);
}

TEST(ScanTest, SingleElementTimeline) {
    auto timeline = timeline_type{std::chrono::seconds(10)};

    add(timeline, 123.45_p, 1'000'000'000);

    spl::metrics::scan::max<feeder::trade::trade_summary> max_scan(timeline);
    spl::metrics::scan::min<feeder::trade::trade_summary> min_scan(timeline);
    spl::metrics::scan::mean<feeder::trade::trade_summary> mean_scan(timeline);
    spl::metrics::scan::median<feeder::trade::trade_summary> median_scan(timeline);

    auto const max_result    = max_scan();
    auto const min_result    = min_scan();
    auto const mean_result   = mean_scan();
    auto const median_result = median_scan();

    EXPECT_EQ(max_result, 123.45_p);
    EXPECT_EQ(min_result, 123.45_p);
    EXPECT_EQ(mean_result, 123.45_p);
    EXPECT_EQ(median_result, 123.45_p);
}
