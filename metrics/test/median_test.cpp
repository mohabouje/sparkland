#include "spl/metrics/timeline.hpp"
#include "spl/metrics/stream/median.hpp"
#include "spl/metrics/scan/median.hpp"
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

template <typename MedianStreamT>
static auto verify_against_scan(timeline_type const& timeline, MedianStreamT const& median_stream, double tolerance = 1e-9) -> void {
    if (timeline.empty()) {
        return;
    }

    auto median_scan = spl::metrics::scan::median<feeder::trade::trade_summary>{const_cast<timeline_type&>(timeline)};

    auto const expected = median_scan();
    auto const result = median_stream();

    ASSERT_TRUE(expected);
    ASSERT_TRUE(result);

    auto const expected_val = static_cast<double>(expected.value());
    auto const result_val = static_cast<double>(result.value());
    auto const diff = std::abs(expected_val - result_val);

    EXPECT_LE(diff, tolerance)
        << "Median mismatch: stream returned " << result_val
        << ", scan returned " << expected_val
        << ", difference = " << diff;
}

TEST(MedianTest, EmptyStreamReturnsFailure) {
    auto const median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    auto const result = median_stream();
    EXPECT_FALSE(result);
}

TEST(MedianTest, SingleElement) {
    auto timeline = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, median_stream);

    auto const result = median_stream();
    ASSERT_TRUE(result);
    EXPECT_EQ(static_cast<double>(result.value()), 100.0);

    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, TwoElementsAverage) {
    auto timeline = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, median_stream);
    add(timeline, 200.0_p, 2'000'000'000, median_stream);

    auto const result = median_stream();
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.value()), 150.0);

    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, ThreeElementsOddCount) {
    auto timeline = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, median_stream);
    add(timeline, 200.0_p, 2'000'000'000, median_stream);
    add(timeline, 300.0_p, 3'000'000'000, median_stream);

    auto const result = median_stream();
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.value()), 200.0);

    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, FourElementsEvenCount) {
    auto timeline = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, median_stream);
    add(timeline, 200.0_p, 2'000'000'000, median_stream);
    add(timeline, 300.0_p, 3'000'000'000, median_stream);
    add(timeline, 400.0_p, 4'000'000'000, median_stream);

    auto const result = median_stream();
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.value()), 250.0);

    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, FiveElementsUnsorted) {
    auto timeline = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 300.0_p, 1'000'000'000, median_stream);
    add(timeline, 100.0_p, 2'000'000'000, median_stream);
    add(timeline, 500.0_p, 3'000'000'000, median_stream);
    add(timeline, 200.0_p, 4'000'000'000, median_stream);
    add(timeline, 400.0_p, 5'000'000'000, median_stream);

    auto const result = median_stream();
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.value()), 300.0);

    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, AscendingSequence) {
    auto timeline = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    for (int i = 1; i <= 9; ++i) {
        add(timeline, spl::types::price::from(static_cast<double>(i * 10)),
            static_cast<int64_t>(i) * 1'000'000'000, median_stream);
    }

    auto const result = median_stream();
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.value()), 50.0);

    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, DescendingSequence) {
    auto timeline = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    for (int i = 9; i >= 1; --i) {
        add(timeline, spl::types::price::from(static_cast<double>(i * 10)),
            static_cast<int64_t>((10 - i)) * 1'000'000'000, median_stream);
    }

    auto const result = median_stream();
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.value()), 50.0);

    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, AllSameValue) {
    auto timeline = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    for (int i = 0; i < 10; ++i) {
        add(timeline, 100.0_p, static_cast<int64_t>(i) * 1'000'000'000, median_stream);
    }

    auto const result = median_stream();
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.value()), 100.0);

    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, RemovalUpdatesMedian) {
    auto timeline = timeline_type{std::chrono::seconds(3)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, median_stream);
    add(timeline, 200.0_p, 2'000'000'000, median_stream);
    add(timeline, 300.0_p, 3'000'000'000, median_stream);
    add(timeline, 400.0_p, 4'000'000'000, median_stream);
    add(timeline, 500.0_p, 5'000'000'000, median_stream);

    auto result = median_stream();
    EXPECT_DOUBLE_EQ(static_cast<double>(result.value()), 300.0);

    auto const now = std::chrono::nanoseconds(9'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) {
        median_stream(begin, end);
    });

    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, RemovalOfMedianElement) {
    auto timeline = timeline_type{std::chrono::seconds(3)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, median_stream);
    add(timeline, 200.0_p, 2'000'000'000, median_stream);
    add(timeline, 300.0_p, 3'000'000'000, median_stream);

    auto result = median_stream();
    EXPECT_DOUBLE_EQ(static_cast<double>(result.value()), 200.0);

    auto const now = std::chrono::nanoseconds(6'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) {
        median_stream(begin, end);
    });

    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, SlidingWindowMaintainsMedian) {
    auto timeline = timeline_type{std::chrono::seconds(5)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, median_stream);
    add(timeline, 200.0_p, 2'000'000'000, median_stream);
    add(timeline, 300.0_p, 3'000'000'000, median_stream);

    verify_against_scan(timeline, median_stream);

    auto now = std::chrono::nanoseconds(9'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) {
        median_stream(begin, end);
    });

    verify_against_scan(timeline, median_stream);

    add(timeline, 250.0_p, 10'000'000'000, median_stream);

    verify_against_scan(timeline, median_stream);

    now = std::chrono::nanoseconds(16'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) {
        median_stream(begin, end);
    });

    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, AlternatingHighLow) {
    auto timeline = timeline_type{std::chrono::seconds(100)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    for (int i = 0; i < 50; ++i) {
        auto const price = (i % 2 == 0) ? 100.0_p : 200.0_p;
        add(timeline, price, static_cast<int64_t>(i) * 1'000'000'000, median_stream);
    }

    auto const result = median_stream();
    EXPECT_DOUBLE_EQ(static_cast<double>(result.value()), 150.0);

    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, RandomSequence100Elements) {
    auto timeline = timeline_type{std::chrono::seconds(200)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    std::mt19937 gen(12345);
    std::uniform_real_distribution<> price_dist(50000.0, 60000.0);

    for (int i = 0; i < 100; ++i) {
        auto const price = spl::types::price::from(price_dist(gen));
        add(timeline, price, static_cast<int64_t>(i) * 1'000'000'000, median_stream);
    }

    verify_against_scan(timeline, median_stream, 1e-6);
}

TEST(MedianTest, RandomSequence1000Elements) {
    auto timeline = timeline_type{std::chrono::seconds(2000)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    std::mt19937 gen(67890);
    std::uniform_real_distribution<> price_dist(40000.0, 70000.0);

    for (int i = 0; i < 1000; ++i) {
        auto const price = spl::types::price::from(price_dist(gen));
        add(timeline, price, static_cast<int64_t>(i) * 1'000'000'000, median_stream);
    }

    verify_against_scan(timeline, median_stream, 1e-3);
}

TEST(MedianTest, VerySmallValues) {
    auto timeline = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 0.001_p, 1'000'000'000, median_stream);
    add(timeline, 0.002_p, 2'000'000'000, median_stream);
    add(timeline, 0.003_p, 3'000'000'000, median_stream);
    add(timeline, 0.004_p, 4'000'000'000, median_stream);
    add(timeline, 0.005_p, 5'000'000'000, median_stream);

    verify_against_scan(timeline, median_stream, 1e-12);
}

TEST(MedianTest, VeryLargeValues) {
    auto timeline = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 1000000.0_p, 1'000'000'000, median_stream);
    add(timeline, 2000000.0_p, 2'000'000'000, median_stream);
    add(timeline, 3000000.0_p, 3'000'000'000, median_stream);
    add(timeline, 4000000.0_p, 4'000'000'000, median_stream);
    add(timeline, 5000000.0_p, 5'000'000'000, median_stream);

    verify_against_scan(timeline, median_stream, 1e-3);
}

TEST(MedianTest, MixedRangeValues) {
    auto timeline = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 0.01_p, 1'000'000'000, median_stream);
    add(timeline, 100.0_p, 2'000'000'000, median_stream);
    add(timeline, 10000.0_p, 3'000'000'000, median_stream);
    add(timeline, 1000000.0_p, 4'000'000'000, median_stream);
    add(timeline, 50.0_p, 5'000'000'000, median_stream);

    verify_against_scan(timeline, median_stream, 1.0);
}

TEST(MedianTest, DuplicateValues) {
    auto timeline = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, median_stream);
    add(timeline, 200.0_p, 2'000'000'000, median_stream);
    add(timeline, 100.0_p, 3'000'000'000, median_stream);
    add(timeline, 200.0_p, 4'000'000'000, median_stream);
    add(timeline, 100.0_p, 5'000'000'000, median_stream);

    auto const result = median_stream();
    EXPECT_DOUBLE_EQ(static_cast<double>(result.value()), 100.0);

    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, MultipleFlushesWithAdditions) {
    auto timeline = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, median_stream);
    add(timeline, 200.0_p, 2'000'000'000, median_stream);
    add(timeline, 300.0_p, 3'000'000'000, median_stream);

    verify_against_scan(timeline, median_stream);

    auto now = std::chrono::nanoseconds(14'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) {
        median_stream(begin, end);
    });

    verify_against_scan(timeline, median_stream);

    add(timeline, 400.0_p, 15'000'000'000, median_stream);
    add(timeline, 500.0_p, 16'000'000'000, median_stream);

    verify_against_scan(timeline, median_stream);

    now = std::chrono::nanoseconds(27'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) {
        median_stream(begin, end);
    });

    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, StressTestWithFlushes) {
    auto timeline = timeline_type{std::chrono::seconds(50)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    std::mt19937 gen(42);
    std::uniform_real_distribution<> price_dist(10000.0, 20000.0);

    for (int i = 0; i < 200; ++i) {
        auto const price = spl::types::price::from(price_dist(gen));
        add(timeline, price, static_cast<int64_t>(i) * 1'000'000'000, median_stream);

        if (i > 0 && i % 50 == 0) {
            auto const now = std::chrono::nanoseconds(static_cast<int64_t>(i + 51) * 1'000'000'000);
            timeline.flush(now, [&](auto begin, auto end) {
                median_stream(begin, end);
            });
            verify_against_scan(timeline, median_stream, 100.0);
        }
    }

    verify_against_scan(timeline, median_stream, 100.0);
}

TEST(MedianTest, SmallIncrementalChanges) {
    auto timeline = timeline_type{std::chrono::seconds(100)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    for (int i = 0; i < 50; ++i) {
        auto const price = spl::types::price::from(100.0 + static_cast<double>(i) * 0.1);
        add(timeline, price, static_cast<int64_t>(i) * 1'000'000'000, median_stream);
    }

    verify_against_scan(timeline, median_stream, 1e-6);
}

TEST(MedianTest, BimodalDistribution) {
    auto timeline = timeline_type{std::chrono::seconds(100)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    for (int i = 0; i < 25; ++i) {
        add(timeline, 100.0_p, static_cast<int64_t>(i * 2) * 1'000'000'000, median_stream);
    }
    for (int i = 0; i < 25; ++i) {
        add(timeline, 200.0_p, static_cast<int64_t>(i * 2 + 1) * 1'000'000'000, median_stream);
    }

    auto const result = median_stream();
    EXPECT_DOUBLE_EQ(static_cast<double>(result.value()), 150.0);

    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, SequentialAddAndRemove) {
    auto timeline = timeline_type{std::chrono::seconds(3)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    for (int cycle = 0; cycle < 10; ++cycle) {
        auto const base_time = static_cast<int64_t>(cycle * 4);

        add(timeline, 100.0_p, (base_time + 0) * 1'000'000'000, median_stream);
        verify_against_scan(timeline, median_stream);

        add(timeline, 200.0_p, (base_time + 1) * 1'000'000'000, median_stream);
        verify_against_scan(timeline, median_stream);

        add(timeline, 300.0_p, (base_time + 2) * 1'000'000'000, median_stream);
        verify_against_scan(timeline, median_stream);

        auto const now = std::chrono::nanoseconds((base_time + 6) * 1'000'000'000);
        timeline.flush(now, [&](auto begin, auto end) {
            median_stream(begin, end);
        });
        verify_against_scan(timeline, median_stream);
    }
}

TEST(MedianTest, HeapBalancing) {
    auto timeline = timeline_type{std::chrono::seconds(20)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 50.0_p, 1'000'000'000, median_stream);
    verify_against_scan(timeline, median_stream);

    add(timeline, 100.0_p, 2'000'000'000, median_stream);
    verify_against_scan(timeline, median_stream);

    add(timeline, 25.0_p, 3'000'000'000, median_stream);
    verify_against_scan(timeline, median_stream);

    add(timeline, 150.0_p, 4'000'000'000, median_stream);
    verify_against_scan(timeline, median_stream);

    add(timeline, 75.0_p, 5'000'000'000, median_stream);
    verify_against_scan(timeline, median_stream);

    add(timeline, 125.0_p, 6'000'000'000, median_stream);
    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, RemovalFromBothHeaps) {
    auto timeline = timeline_type{std::chrono::seconds(5)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, median_stream);
    add(timeline, 200.0_p, 2'000'000'000, median_stream);
    add(timeline, 300.0_p, 3'000'000'000, median_stream);
    add(timeline, 400.0_p, 4'000'000'000, median_stream);
    add(timeline, 50.0_p, 5'000'000'000, median_stream);
    add(timeline, 350.0_p, 6'000'000'000, median_stream);

    verify_against_scan(timeline, median_stream, 100.0);

    auto const now = std::chrono::nanoseconds(12'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) {
        median_stream(begin, end);
    });

    verify_against_scan(timeline, median_stream, 100.0);
}

TEST(MedianTest, OddToEvenTransition) {
    auto timeline = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, median_stream);
    add(timeline, 200.0_p, 2'000'000'000, median_stream);
    add(timeline, 300.0_p, 3'000'000'000, median_stream);

    auto result = median_stream();
    EXPECT_DOUBLE_EQ(static_cast<double>(result.value()), 200.0);

    add(timeline, 400.0_p, 4'000'000'000, median_stream);

    result = median_stream();
    EXPECT_DOUBLE_EQ(static_cast<double>(result.value()), 250.0);

    verify_against_scan(timeline, median_stream);
}

TEST(MedianTest, EvenToOddTransition) {
    auto timeline = timeline_type{std::chrono::seconds(10)};
    auto median_stream = spl::metrics::stream::median<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, median_stream);
    add(timeline, 200.0_p, 2'000'000'000, median_stream);
    add(timeline, 300.0_p, 3'000'000'000, median_stream);
    add(timeline, 400.0_p, 4'000'000'000, median_stream);

    auto result = median_stream();
    EXPECT_DOUBLE_EQ(static_cast<double>(result.value()), 250.0);

    add(timeline, 500.0_p, 5'000'000'000, median_stream);

    result = median_stream();
    EXPECT_DOUBLE_EQ(static_cast<double>(result.value()), 300.0);

    verify_against_scan(timeline, median_stream);
}
