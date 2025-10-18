#include "spl/metrics/timeline.hpp"
#include "spl/metrics/stream/mean.hpp"
#include "spl/metrics/scan/mean.hpp"
#include "spl/protocol/feeder/trade/trade_summary.hpp"

#include <gtest/gtest.h>
#include <random>
#include <cmath>

using namespace spl::protocol;

using timeline_type = spl::metrics::timeline<feeder::trade::trade_summary>;

constexpr auto add(timeline_type& timeline, spl::types::price price, int64_t timestamp) -> void {
    std::ignore = timeline.emplace_back(feeder::trade::trade_summary{
        .price     = price,
        .timestamp = std::chrono::nanoseconds(timestamp),
    });
}

template <typename... MetricsT>
constexpr auto add(timeline_type& timeline, spl::types::price price, int64_t timestamp, MetricsT&... metrics) -> void {
    auto const& trade = timeline.emplace_back(feeder::trade::trade_summary{
        .price     = price,
        .timestamp = std::chrono::nanoseconds(timestamp),
    });
    (metrics(trade), ...);
}

template <typename MeanStreamT>
static auto verify_against_scan(timeline_type const& timeline, MeanStreamT const& mean_stream, double tolerance = 1e-9)
    -> void {
    if (timeline.empty()) {
        return;
    }

    auto mean_scan = spl::metrics::scan::mean<feeder::trade::trade_summary>{const_cast<timeline_type&>(timeline)};

    auto const expected = mean_scan();
    auto const result   = mean_stream();

    auto const expected_val = static_cast<double>(expected);
    auto const result_val   = static_cast<double>(result);
    auto const diff         = std::abs(expected_val - result_val);

    EXPECT_LE(diff, tolerance) << "Mean mismatch: stream returned " << result_val << ", scan returned " << expected_val
                               << ", difference = " << diff;
}

TEST(MeanTest, SingleElement) {
    auto timeline    = timeline_type{std::chrono::seconds(10)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, mean_stream);

    auto const result = mean_stream();

    EXPECT_EQ(static_cast<double>(result), 100.0);

    verify_against_scan(timeline, mean_stream);
}

TEST(MeanTest, TwoElementsSimpleAverage) {
    auto timeline    = timeline_type{std::chrono::seconds(10)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, mean_stream);
    add(timeline, 200.0_p, 2'000'000'000, mean_stream);

    auto const result = mean_stream();

    EXPECT_DOUBLE_EQ(static_cast<double>(result), 150.0);

    verify_against_scan(timeline, mean_stream);
}

TEST(MeanTest, ThreeElementsAverage) {
    auto timeline    = timeline_type{std::chrono::seconds(10)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, mean_stream);
    add(timeline, 200.0_p, 2'000'000'000, mean_stream);
    add(timeline, 300.0_p, 3'000'000'000, mean_stream);

    auto const result = mean_stream();

    EXPECT_DOUBLE_EQ(static_cast<double>(result), 200.0);

    verify_against_scan(timeline, mean_stream);
}

TEST(MeanTest, MultipleIdenticalValues) {
    auto timeline    = timeline_type{std::chrono::seconds(10)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, mean_stream);
    add(timeline, 100.0_p, 2'000'000'000, mean_stream);
    add(timeline, 100.0_p, 3'000'000'000, mean_stream);
    add(timeline, 100.0_p, 4'000'000'000, mean_stream);

    auto const result = mean_stream();

    EXPECT_DOUBLE_EQ(static_cast<double>(result), 100.0);

    verify_against_scan(timeline, mean_stream);
}

TEST(MeanTest, IncreasingSequence) {
    auto timeline    = timeline_type{std::chrono::seconds(10)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    for (int i = 1; i <= 10; ++i) {
        add(timeline, spl::types::price::from(static_cast<double>(i * 10)), static_cast<int64_t>(i) * 1'000'000'000,
            mean_stream);
    }

    verify_against_scan(timeline, mean_stream);
}

TEST(MeanTest, DecreasingSequence) {
    auto timeline    = timeline_type{std::chrono::seconds(10)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    for (int i = 10; i >= 1; --i) {
        add(timeline, spl::types::price::from(static_cast<double>(i * 10)),
            static_cast<int64_t>((11 - i)) * 1'000'000'000, mean_stream);
    }

    verify_against_scan(timeline, mean_stream);
}

TEST(MeanTest, RemovalUpdatesCorrectly) {
    auto timeline    = timeline_type{std::chrono::seconds(3)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, mean_stream);
    add(timeline, 200.0_p, 2'000'000'000, mean_stream);
    add(timeline, 300.0_p, 3'000'000'000, mean_stream);

    auto result = mean_stream();
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 200.0);

    auto const now = std::chrono::nanoseconds(4'500'000'000);
    timeline.flush(now, [&](auto begin, auto end) { mean_stream(begin, end); });

    result = mean_stream();
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 250.0);

    verify_against_scan(timeline, mean_stream);
}

TEST(MeanTest, RemovalOfAllElementsThenAdd) {
    auto timeline    = timeline_type{std::chrono::seconds(2)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, mean_stream);
    add(timeline, 200.0_p, 2'000'000'000, mean_stream);

    auto const now = std::chrono::nanoseconds(4'500'000'000);
    timeline.flush(now, [&](auto begin, auto end) { mean_stream(begin, end); });

    add(timeline, 300.0_p, 5'000'000'000, mean_stream);

    auto const result = mean_stream();
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 300.0);

    verify_against_scan(timeline, mean_stream);
}

TEST(MeanTest, SlidingWindowMaintainsMean) {
    auto timeline    = timeline_type{std::chrono::seconds(5)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, mean_stream);
    add(timeline, 200.0_p, 2'000'000'000, mean_stream);
    add(timeline, 150.0_p, 3'000'000'000, mean_stream);

    verify_against_scan(timeline, mean_stream);

    auto now = std::chrono::nanoseconds(9'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) { mean_stream(begin, end); });

    verify_against_scan(timeline, mean_stream);

    add(timeline, 250.0_p, 10'000'000'000, mean_stream);

    verify_against_scan(timeline, mean_stream);

    now = std::chrono::nanoseconds(16'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) { mean_stream(begin, end); });

    verify_against_scan(timeline, mean_stream);
}

TEST(MeanTest, LargeNumberOfElements) {
    auto timeline    = timeline_type{std::chrono::seconds(100)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    for (int i = 0; i < 100; ++i) {
        add(timeline, spl::types::price::from(static_cast<double>(i + 1)), static_cast<int64_t>(i) * 1'000'000'000,
            mean_stream);
    }

    verify_against_scan(timeline, mean_stream, 1e-6);
}

TEST(MeanTest, RandomSequence100Elements) {
    auto timeline    = timeline_type{std::chrono::seconds(200)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    std::mt19937 gen(12345);
    std::uniform_real_distribution<> price_dist(50000.0, 60000.0);

    for (int i = 0; i < 100; ++i) {
        auto const price = spl::types::price::from(price_dist(gen));
        add(timeline, price, static_cast<int64_t>(i) * 1'000'000'000, mean_stream);
    }

    verify_against_scan(timeline, mean_stream, 1e-6);
}

TEST(MeanTest, RandomSequence1000Elements) {
    auto timeline    = timeline_type{std::chrono::seconds(2000)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    std::mt19937 gen(67890);
    std::uniform_real_distribution<> price_dist(40000.0, 70000.0);

    for (int i = 0; i < 1000; ++i) {
        auto const price = spl::types::price::from(price_dist(gen));
        add(timeline, price, static_cast<int64_t>(i) * 1'000'000'000, mean_stream);
    }

    verify_against_scan(timeline, mean_stream, 1e-3);
}

TEST(MeanTest, VerySmallValues) {
    auto timeline    = timeline_type{std::chrono::seconds(10)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    add(timeline, 0.001_p, 1'000'000'000, mean_stream);
    add(timeline, 0.002_p, 2'000'000'000, mean_stream);
    add(timeline, 0.003_p, 3'000'000'000, mean_stream);

    verify_against_scan(timeline, mean_stream, 1e-12);
}

TEST(MeanTest, VeryLargeValues) {
    auto timeline    = timeline_type{std::chrono::seconds(10)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    add(timeline, 1000000.0_p, 1'000'000'000, mean_stream);
    add(timeline, 2000000.0_p, 2'000'000'000, mean_stream);
    add(timeline, 3000000.0_p, 3'000'000'000, mean_stream);

    verify_against_scan(timeline, mean_stream, 1e-3);
}

TEST(MeanTest, MixedPositiveValues) {
    auto timeline    = timeline_type{std::chrono::seconds(10)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    add(timeline, 0.01_p, 1'000'000'000, mean_stream);
    add(timeline, 100.0_p, 2'000'000'000, mean_stream);
    add(timeline, 10000.0_p, 3'000'000'000, mean_stream);
    add(timeline, 1000000.0_p, 4'000'000'000, mean_stream);

    verify_against_scan(timeline, mean_stream, 1.0);
}

TEST(MeanTest, AccumulationAccuracy) {
    auto timeline    = timeline_type{std::chrono::seconds(1000)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    for (int i = 0; i < 1000; ++i) {
        add(timeline, 100.123456789_p, static_cast<int64_t>(i) * 1'000'000'000, mean_stream);
    }

    auto const result = mean_stream();

    EXPECT_NEAR(static_cast<double>(result), 100.123456789, 1e-6);

    verify_against_scan(timeline, mean_stream, 1e-6);
}

TEST(MeanTest, AlternatingHighLow) {
    auto timeline    = timeline_type{std::chrono::seconds(100)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    for (int i = 0; i < 50; ++i) {
        auto const price = (i % 2 == 0) ? 100.0_p : 200.0_p;
        add(timeline, price, static_cast<int64_t>(i) * 1'000'000'000, mean_stream);
    }

    auto const result = mean_stream();
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 150.0);

    verify_against_scan(timeline, mean_stream);
}

TEST(MeanTest, MultipleFlushesWithAdditions) {
    auto timeline    = timeline_type{std::chrono::seconds(10)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, mean_stream);
    add(timeline, 200.0_p, 2'000'000'000, mean_stream);
    add(timeline, 300.0_p, 3'000'000'000, mean_stream);

    verify_against_scan(timeline, mean_stream);

    auto now = std::chrono::nanoseconds(14'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) { mean_stream(begin, end); });

    verify_against_scan(timeline, mean_stream);

    add(timeline, 400.0_p, 15'000'000'000, mean_stream);
    add(timeline, 500.0_p, 16'000'000'000, mean_stream);

    verify_against_scan(timeline, mean_stream);

    now = std::chrono::nanoseconds(27'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) { mean_stream(begin, end); });

    verify_against_scan(timeline, mean_stream);
}

TEST(MeanTest, StressTestWithFlushes) {
    auto timeline    = timeline_type{std::chrono::seconds(50)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    std::mt19937 gen(42);
    std::uniform_real_distribution<> price_dist(10000.0, 20000.0);

    for (int i = 0; i < 200; ++i) {
        auto const price = spl::types::price::from(price_dist(gen));
        add(timeline, price, static_cast<int64_t>(i) * 1'000'000'000, mean_stream);

        if (i > 0 && i % 50 == 0) {
            auto const now = std::chrono::nanoseconds(static_cast<int64_t>(i + 51) * 1'000'000'000);
            timeline.flush(now, [&](auto begin, auto end) { mean_stream(begin, end); });
            verify_against_scan(timeline, mean_stream, 100.0);
        }
    }

    verify_against_scan(timeline, mean_stream, 100.0);
}

TEST(MeanTest, PrecisionWithManySmallIncrements) {
    auto timeline    = timeline_type{std::chrono::seconds(100)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    for (int i = 0; i < 100; ++i) {
        auto const price = spl::types::price::from(100.0 + static_cast<double>(i) * 0.01);
        add(timeline, price, static_cast<int64_t>(i) * 1'000'000'000, mean_stream);
    }

    verify_against_scan(timeline, mean_stream, 1e-6);
}

TEST(MeanTest, RemovalInMiddleOfSequence) {
    auto timeline    = timeline_type{std::chrono::seconds(5)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, mean_stream);
    add(timeline, 200.0_p, 3'000'000'000, mean_stream);
    add(timeline, 300.0_p, 5'000'000'000, mean_stream);

    verify_against_scan(timeline, mean_stream);

    auto const now = std::chrono::nanoseconds(6'500'000'000);
    timeline.flush(now, [&](auto begin, auto end) { mean_stream(begin, end); });

    verify_against_scan(timeline, mean_stream);
}

TEST(MeanTest, SequentialAddAndRemove) {
    auto timeline    = timeline_type{std::chrono::seconds(3)};
    auto mean_stream = spl::metrics::stream::mean<feeder::trade::trade_summary>{};

    for (int cycle = 0; cycle < 10; ++cycle) {
        auto const base_time = static_cast<int64_t>(cycle * 4);

        add(timeline, 100.0_p, (base_time + 0) * 1'000'000'000, mean_stream);
        verify_against_scan(timeline, mean_stream);

        add(timeline, 200.0_p, (base_time + 1) * 1'000'000'000, mean_stream);
        verify_against_scan(timeline, mean_stream);

        add(timeline, 300.0_p, (base_time + 2) * 1'000'000'000, mean_stream);
        verify_against_scan(timeline, mean_stream);

        auto const now = std::chrono::nanoseconds((base_time + 6) * 1'000'000'000);
        timeline.flush(now, [&](auto begin, auto end) { mean_stream(begin, end); });
        verify_against_scan(timeline, mean_stream);
    }
}
