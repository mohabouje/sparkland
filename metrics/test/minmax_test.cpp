#include "spl/metrics/timeline.hpp"
#include "spl/metrics/stream/max.hpp"
#include "spl/metrics/stream/min.hpp"
#include "spl/metrics/scan/max.hpp"
#include "spl/metrics/scan/min.hpp"
#include "spl/protocol/feeder/trade/trade_summary.hpp"

#include <gtest/gtest.h>
#include <random>

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

template <typename MinStreamT, typename MaxStreamT>
static auto verify_against_scan(timeline_type const& timeline, MinStreamT const& min_stream,
                                MaxStreamT const& max_stream) -> void {
    if (timeline.empty()) {
        EXPECT_FALSE(min_stream());
        EXPECT_FALSE(max_stream());
        return;
    }

    auto min_scan = spl::metrics::scan::min<feeder::trade::trade_summary>{const_cast<timeline_type&>(timeline)};
    auto max_scan = spl::metrics::scan::max<feeder::trade::trade_summary>{const_cast<timeline_type&>(timeline)};

    auto const min_expected = min_scan();
    auto const max_expected = max_scan();

    auto const min_result = min_stream();
    auto const max_result = max_stream();

    ASSERT_TRUE(min_expected);
    ASSERT_TRUE(max_expected);
    ASSERT_TRUE(min_result);
    ASSERT_TRUE(max_result);

    EXPECT_EQ(min_result.value(), min_expected.value())
        << "Min mismatch: stream returned " << static_cast<double>(min_result.value()) << ", scan returned "
        << static_cast<double>(min_expected.value());

    EXPECT_EQ(max_result.value(), max_expected.value())
        << "Max mismatch: stream returned " << static_cast<double>(max_result.value()) << ", scan returned "
        << static_cast<double>(max_expected.value());
}

TEST(MinMaxTest, SingleElement) {
    auto timeline   = timeline_type{std::chrono::seconds(10)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, min_stream, max_stream);

    auto const min_result = min_stream();
    auto const max_result = max_stream();

    ASSERT_TRUE(min_result);
    ASSERT_TRUE(max_result);
    EXPECT_EQ(min_result.value(), 100.0_p);
    EXPECT_EQ(max_result.value(), 100.0_p);

    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, TwoElementsAscending) {
    auto timeline   = timeline_type{std::chrono::seconds(10)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, min_stream, max_stream);
    add(timeline, 200.0_p, 2'000'000'000, min_stream, max_stream);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, TwoElementsDescending) {
    auto timeline   = timeline_type{std::chrono::seconds(10)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 200.0_p, 1'000'000'000, min_stream, max_stream);
    add(timeline, 100.0_p, 2'000'000'000, min_stream, max_stream);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, StrictlyAscendingSequencePopsAllPreviousForMax) {
    auto timeline   = timeline_type{std::chrono::seconds(10)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, min_stream, max_stream);
    add(timeline, 110.0_p, 2'000'000'000, min_stream, max_stream);
    add(timeline, 120.0_p, 3'000'000'000, min_stream, max_stream);
    add(timeline, 130.0_p, 4'000'000'000, min_stream, max_stream);
    add(timeline, 140.0_p, 5'000'000'000, min_stream, max_stream);

    EXPECT_EQ(max_stream().value(), 140.0_p);
    EXPECT_EQ(min_stream().value(), 100.0_p);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, StrictlyDescendingSequencePopsAllPreviousForMin) {
    auto timeline   = timeline_type{std::chrono::seconds(10)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 140.0_p, 1'000'000'000, min_stream, max_stream);
    add(timeline, 130.0_p, 2'000'000'000, min_stream, max_stream);
    add(timeline, 120.0_p, 3'000'000'000, min_stream, max_stream);
    add(timeline, 110.0_p, 4'000'000'000, min_stream, max_stream);
    add(timeline, 100.0_p, 5'000'000'000, min_stream, max_stream);

    EXPECT_EQ(max_stream().value(), 140.0_p);
    EXPECT_EQ(min_stream().value(), 100.0_p);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, MountainPatternPreservesDequeInvariant) {
    auto timeline   = timeline_type{std::chrono::seconds(10)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, min_stream, max_stream);
    add(timeline, 110.0_p, 2'000'000'000, min_stream, max_stream);
    add(timeline, 120.0_p, 3'000'000'000, min_stream, max_stream);
    add(timeline, 115.0_p, 4'000'000'000, min_stream, max_stream);
    add(timeline, 105.0_p, 5'000'000'000, min_stream, max_stream);

    EXPECT_EQ(max_stream().value(), 120.0_p);
    EXPECT_EQ(min_stream().value(), 100.0_p);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, ValleyPatternPreservesDequeInvariant) {
    auto timeline   = timeline_type{std::chrono::seconds(10)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 120.0_p, 1'000'000'000, min_stream, max_stream);
    add(timeline, 110.0_p, 2'000'000'000, min_stream, max_stream);
    add(timeline, 100.0_p, 3'000'000'000, min_stream, max_stream);
    add(timeline, 105.0_p, 4'000'000'000, min_stream, max_stream);
    add(timeline, 115.0_p, 5'000'000'000, min_stream, max_stream);

    EXPECT_EQ(max_stream().value(), 120.0_p);
    EXPECT_EQ(min_stream().value(), 100.0_p);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, DuplicatePricesIncrementCount) {
    auto timeline   = timeline_type{std::chrono::seconds(10)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, min_stream, max_stream);
    add(timeline, 100.0_p, 2'000'000'000, min_stream, max_stream);
    add(timeline, 100.0_p, 3'000'000'000, min_stream, max_stream);

    EXPECT_EQ(max_stream().value(), 100.0_p);
    EXPECT_EQ(min_stream().value(), 100.0_p);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, DuplicatesAtDifferentPositions) {
    auto timeline   = timeline_type{std::chrono::seconds(10)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, min_stream, max_stream);
    add(timeline, 110.0_p, 2'000'000'000, min_stream, max_stream);
    add(timeline, 110.0_p, 3'000'000'000, min_stream, max_stream);
    add(timeline, 110.0_p, 4'000'000'000, min_stream, max_stream);
    add(timeline, 105.0_p, 5'000'000'000, min_stream, max_stream);

    EXPECT_EQ(max_stream().value(), 110.0_p);
    EXPECT_EQ(min_stream().value(), 100.0_p);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, NewMaximumPopsAllSmallerElements) {
    auto timeline   = timeline_type{std::chrono::seconds(10)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, min_stream, max_stream);
    add(timeline, 90.0_p, 2'000'000'000, min_stream, max_stream);
    add(timeline, 95.0_p, 3'000'000'000, min_stream, max_stream);
    add(timeline, 85.0_p, 4'000'000'000, min_stream, max_stream);

    EXPECT_EQ(max_stream().value(), 100.0_p);

    add(timeline, 110.0_p, 5'000'000'000, min_stream, max_stream);

    EXPECT_EQ(max_stream().value(), 110.0_p);
    EXPECT_EQ(min_stream().value(), 85.0_p);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, NewMinimumPopsAllLargerElements) {
    auto timeline   = timeline_type{std::chrono::seconds(10)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, min_stream, max_stream);
    add(timeline, 110.0_p, 2'000'000'000, min_stream, max_stream);
    add(timeline, 105.0_p, 3'000'000'000, min_stream, max_stream);
    add(timeline, 115.0_p, 4'000'000'000, min_stream, max_stream);

    EXPECT_EQ(min_stream().value(), 100.0_p);

    add(timeline, 90.0_p, 5'000'000'000, min_stream, max_stream);

    EXPECT_EQ(max_stream().value(), 115.0_p);
    EXPECT_EQ(min_stream().value(), 90.0_p);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, RemovalOfMaximumUpdatesCorrectly) {
    auto timeline   = timeline_type{std::chrono::seconds(3)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, min_stream, max_stream);
    add(timeline, 200.0_p, 2'000'000'000, min_stream, max_stream);
    add(timeline, 150.0_p, 3'000'000'000, min_stream, max_stream);

    EXPECT_EQ(max_stream().value(), 200.0_p);

    auto const now = std::chrono::nanoseconds(5'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) {
        min_stream(begin, end);
        max_stream(begin, end);
    });

    EXPECT_EQ(max_stream().value(), 150.0_p);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, RemovalOfMinimumUpdatesCorrectly) {
    auto timeline   = timeline_type{std::chrono::seconds(3)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, min_stream, max_stream);
    add(timeline, 200.0_p, 2'000'000'000, min_stream, max_stream);
    add(timeline, 150.0_p, 3'000'000'000, min_stream, max_stream);

    EXPECT_EQ(min_stream().value(), 100.0_p);

    auto const now = std::chrono::nanoseconds(4'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) {
        min_stream(begin, end);
        max_stream(begin, end);
    });

    EXPECT_EQ(min_stream().value(), 150.0_p);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, RemovalOfDuplicatesDecrementsCount) {
    auto timeline   = timeline_type{std::chrono::seconds(5)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, min_stream, max_stream);
    add(timeline, 100.0_p, 2'000'000'000, min_stream, max_stream);
    add(timeline, 100.0_p, 3'000'000'000, min_stream, max_stream);
    add(timeline, 90.0_p, 4'000'000'000, min_stream, max_stream);

    EXPECT_EQ(max_stream().value(), 100.0_p);
    EXPECT_EQ(min_stream().value(), 90.0_p);

    auto now = std::chrono::nanoseconds(6'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) {
        min_stream(begin, end);
        max_stream(begin, end);
    });

    EXPECT_EQ(max_stream().value(), 100.0_p);

    now = std::chrono::nanoseconds(7'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) {
        min_stream(begin, end);
        max_stream(begin, end);
    });

    EXPECT_EQ(max_stream().value(), 100.0_p);

    now = std::chrono::nanoseconds(8'000'000'000);
    timeline.flush(now, [&](auto begin, auto end) {
        min_stream(begin, end);
        max_stream(begin, end);
    });

    EXPECT_EQ(max_stream().value(), 90.0_p);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, RandomSequence100Elements) {
    auto timeline   = timeline_type{std::chrono::seconds(200)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    std::mt19937 gen(12345);
    std::uniform_real_distribution<> price_dist(50000.0, 60000.0);

    for (int i = 0; i < 100; ++i) {
        auto const price = spl::types::price::from(price_dist(gen));
        add(timeline, price, static_cast<int64_t>(i) * 1'000'000'000);
        min_stream(timeline.back());
        max_stream(timeline.back());
    }

    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, RandomSequence1000Elements) {
    auto timeline   = timeline_type{std::chrono::seconds(2000)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    std::mt19937 gen(67890);
    std::uniform_real_distribution<> price_dist(40000.0, 70000.0);

    for (int i = 0; i < 1000; ++i) {
        auto const price = spl::types::price::from(price_dist(gen));
        add(timeline, price, static_cast<int64_t>(i) * 1'000'000'000);
        min_stream(timeline.back());
        max_stream(timeline.back());
    }

    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, AlternatingHighLowPattern) {
    auto timeline   = timeline_type{std::chrono::seconds(100)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    for (int i = 0; i < 50; ++i) {
        auto const price = (i % 2 == 0) ? 100.0_p : 200.0_p;
        add(timeline, price, static_cast<int64_t>(i) * 1'000'000'000);
        min_stream(timeline.back());
        max_stream(timeline.back());
    }

    EXPECT_EQ(max_stream().value(), 200.0_p);
    EXPECT_EQ(min_stream().value(), 100.0_p);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, SawtoothPatternAscending) {
    auto timeline   = timeline_type{std::chrono::seconds(100)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    for (int cycle = 0; cycle < 10; ++cycle) {
        add(timeline, 100.0_p, static_cast<int64_t>(cycle * 3 + 0) * 1'000'000'000);
        min_stream(timeline.back());
        max_stream(timeline.back());

        add(timeline, 110.0_p, static_cast<int64_t>(cycle * 3 + 1) * 1'000'000'000);
        min_stream(timeline.back());
        max_stream(timeline.back());

        add(timeline, 120.0_p, static_cast<int64_t>(cycle * 3 + 2) * 1'000'000'000);
        min_stream(timeline.back());
        max_stream(timeline.back());
    }

    EXPECT_EQ(max_stream().value(), 120.0_p);
    EXPECT_EQ(min_stream().value(), 100.0_p);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, SawtoothPatternDescending) {
    auto timeline   = timeline_type{std::chrono::seconds(100)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    for (int cycle = 0; cycle < 10; ++cycle) {
        add(timeline, 120.0_p, static_cast<int64_t>(cycle * 3 + 0) * 1'000'000'000);
        min_stream(timeline.back());
        max_stream(timeline.back());

        add(timeline, 110.0_p, static_cast<int64_t>(cycle * 3 + 1) * 1'000'000'000);
        min_stream(timeline.back());
        max_stream(timeline.back());

        add(timeline, 100.0_p, static_cast<int64_t>(cycle * 3 + 2) * 1'000'000'000);
        min_stream(timeline.back());
        max_stream(timeline.back());
    }

    EXPECT_EQ(max_stream().value(), 120.0_p);
    EXPECT_EQ(min_stream().value(), 100.0_p);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, AllSamePrice) {
    auto timeline   = timeline_type{std::chrono::seconds(200)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    for (int i = 0; i < 100; ++i) {
        add(timeline, 100.0_p, static_cast<int64_t>(i) * 1'000'000'000);
        min_stream(timeline.back());
        max_stream(timeline.back());
    }

    EXPECT_EQ(max_stream().value(), 100.0_p);
    EXPECT_EQ(min_stream().value(), 100.0_p);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, VerySmallPriceChanges) {
    auto timeline   = timeline_type{std::chrono::seconds(10)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 100.000000_p, 1'000'000'000, min_stream, max_stream);
    add(timeline, 100.000001_p, 2'000'000'000, min_stream, max_stream);
    add(timeline, 100.000002_p, 3'000'000'000, min_stream, max_stream);
    add(timeline, 100.000001_p, 4'000'000'000, min_stream, max_stream);
    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, ExtremePriceRange) {
    auto timeline   = timeline_type{std::chrono::seconds(10)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 0.01_p, 1'000'000'000, min_stream, max_stream);
    add(timeline, 1000000.0_p, 2'000'000'000, min_stream, max_stream);
    add(timeline, 500.0_p, 3'000'000'000, min_stream, max_stream);

    EXPECT_EQ(min_stream().value(), 0.01_p);
    EXPECT_EQ(max_stream().value(), 1000000.0_p);

    verify_against_scan(timeline, min_stream, max_stream);
}

TEST(MinMaxTest, ComplexScenarioWithMultipleFlushes) {
    auto timeline   = timeline_type{std::chrono::seconds(10)};
    auto min_stream = spl::metrics::stream::min<feeder::trade::trade_summary>{};
    auto max_stream = spl::metrics::stream::max<feeder::trade::trade_summary>{};

    add(timeline, 100.0_p, 1'000'000'000, min_stream, max_stream);
    add(timeline, 200.0_p, 2'000'000'000, min_stream, max_stream);
    add(timeline, 150.0_p, 3'000'000'000, min_stream, max_stream);
    verify_against_scan(timeline, min_stream, max_stream);

    add(timeline, 50.0_p, 5'000'000'000, min_stream, max_stream);
    add(timeline, 250.0_p, 6'000'000'000, min_stream, max_stream);
    verify_against_scan(timeline, min_stream, max_stream);

    auto const t1 = std::chrono::nanoseconds(12'000'000'000);
    timeline.flush(t1, [&](auto begin, auto end) {
        min_stream(begin, end);
        max_stream(begin, end);
    });

    verify_against_scan(timeline, min_stream, max_stream);
    add(timeline, 175.0_p, 13'000'000'000, min_stream, max_stream);
    verify_against_scan(timeline, min_stream, max_stream);

    auto const t2 = std::chrono::nanoseconds(17'000'000'000);
    timeline.flush(t2, [&](auto begin, auto end) {
        min_stream(begin, end);
        max_stream(begin, end);
    });
    verify_against_scan(timeline, min_stream, max_stream);
}
