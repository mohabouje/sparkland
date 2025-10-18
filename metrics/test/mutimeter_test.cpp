#include "spl/metrics/scan/multimeter.hpp"
#include "spl/metrics/stream/multimeter.hpp"
#include "spl/protocol/feeder/trade/trade_summary.hpp"
#include "spl/protocol/common/exchange_id.hpp"
#include "spl/protocol/common/instrument_id.hpp"
#include "spl/protocol/common/trade_id.hpp"
#include "spl/protocol/common/aggressor_side.hpp"
#include "spl/protocol/common/price.hpp"
#include "spl/protocol/common/quantity.hpp"
#include "spl/protocol/common/trade_condition.hpp"
#include "spl/protocol/common/sequence.hpp"
#include "spl/protocol/common/timestamp.hpp"

#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <cmath>
#include <memory>

using trade_summary = spl::protocol::feeder::trade::trade_summary;

// Test fixture template for both multimeter types
template <typename MultimeterType>
class MultimeterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use 500ms window for testing
        multimeter_ = std::make_unique<MultimeterType>(std::chrono::milliseconds{500});
    }

    // Helper to create test trade data
    trade_summary create_trade(double price, std::uint64_t sequence, std::chrono::nanoseconds timestamp) {
        return trade_summary{.instrument_id = spl::protocol::common::instrument_id{1},
                             .exchange_id   = spl::protocol::common::exchange_id::bybit,
                             .side          = spl::protocol::common::aggressor_side::buy,
                             .price         = spl::protocol::common::price::from(price),
                             .quantity      = spl::protocol::common::quantity::from(1.0),
                             .sequence      = spl::protocol::common::sequence{sequence},
                             .timestamp     = spl::protocol::common::timestamp{timestamp}};
    }

    std::unique_ptr<MultimeterType> multimeter_;
};

// Define the types to test
using ScanMultimeter   = spl::metrics::scan::multimeter<trade_summary>;
using StreamMultimeter = spl::metrics::stream::multimeter<trade_summary>;
using MultimeterTypes  = ::testing::Types<ScanMultimeter, StreamMultimeter>;

TYPED_TEST_SUITE(MultimeterTest, MultimeterTypes);

TYPED_TEST(MultimeterTest, SingleTradeMetrics) {
    auto base_time = std::chrono::steady_clock::now().time_since_epoch();
    auto trade     = this->create_trade(100.50, 1, base_time);

    auto result = (*this->multimeter_)(trade);

    EXPECT_DOUBLE_EQ(static_cast<double>(result.minimum), 100.50);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.maximum), 100.50);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.median), 100.50);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.mean), 100.50);
}

TYPED_TEST(MultimeterTest, MultipleTradesBasicStats) {
    auto base_time = std::chrono::steady_clock::now().time_since_epoch();

    // Add trades with prices: 100, 200, 300
    std::vector<double> prices = {100.0, 200.0, 300.0};
    spl::metrics::metrics last_result;

    for (size_t i = 0; i < prices.size(); ++i) {
        auto trade  = this->create_trade(prices[i], i + 1, base_time + std::chrono::milliseconds{i * 10});
        last_result = (*this->multimeter_)(trade);
    }

    // After all trades: min=100, max=300, median=200, mean=200
    EXPECT_DOUBLE_EQ(static_cast<double>(last_result.minimum), 100.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(last_result.maximum), 300.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(last_result.median), 200.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(last_result.mean), 200.0);
}

TYPED_TEST(MultimeterTest, OddNumberOfTradesMedian) {
    auto base_time = std::chrono::steady_clock::now().time_since_epoch();

    // Add 5 trades with prices: 10, 30, 20, 50, 40
    std::vector<double> prices = {10.0, 30.0, 20.0, 50.0, 40.0};
    spl::metrics::metrics last_result;

    for (size_t i = 0; i < prices.size(); ++i) {
        auto trade  = this->create_trade(prices[i], i + 1, base_time + std::chrono::milliseconds{i * 10});
        last_result = (*this->multimeter_)(trade);
    }

    // Sorted: 10, 20, 30, 40, 50 -> median=30, mean=30
    EXPECT_DOUBLE_EQ(static_cast<double>(last_result.minimum), 10.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(last_result.maximum), 50.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(last_result.median), 30.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(last_result.mean), 30.0);
}

TYPED_TEST(MultimeterTest, EvenNumberOfTradesMedian) {
    auto base_time = std::chrono::steady_clock::now().time_since_epoch();

    // Add 4 trades with prices: 10, 20, 30, 40
    std::vector<double> prices = {10.0, 20.0, 30.0, 40.0};
    spl::metrics::metrics last_result;

    for (size_t i = 0; i < prices.size(); ++i) {
        auto trade  = this->create_trade(prices[i], i + 1, base_time + std::chrono::milliseconds{i * 10});
        last_result = (*this->multimeter_)(trade);
    }

    // Sorted: 10, 20, 30, 40 -> median=(20+30)/2=25, mean=25
    EXPECT_DOUBLE_EQ(static_cast<double>(last_result.minimum), 10.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(last_result.maximum), 40.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(last_result.median), 25.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(last_result.mean), 25.0);
}

// Cross-implementation consistency test using typed test
template <typename MultimeterType>
class MultimeterConsistencyTest : public ::testing::Test {
protected:
    void SetUp() override {
        multimeter_ = std::make_unique<MultimeterType>(std::chrono::milliseconds{1000});
    }

    trade_summary create_trade(double price, std::uint64_t sequence, std::chrono::nanoseconds timestamp) {
        return trade_summary{.instrument_id = spl::protocol::common::instrument_id{1},
                             .exchange_id   = spl::protocol::common::exchange_id::coinbase,
                             .trade_id      = spl::protocol::common::trade_id{static_cast<char>(sequence)},
                             .side          = spl::protocol::common::aggressor_side::sell,
                             .price         = spl::protocol::common::price::from(price),
                             .quantity      = spl::protocol::common::quantity::from(1.0),
                             .condition     = spl::protocol::common::trade_condition{},
                             .sequence      = spl::protocol::common::sequence{sequence},
                             .timestamp     = spl::protocol::common::timestamp{timestamp}};
    }

    std::unique_ptr<MultimeterType> multimeter_;
};

TYPED_TEST_SUITE(MultimeterConsistencyTest, MultimeterTypes);

TYPED_TEST(MultimeterConsistencyTest, RealisticTradeStream) {
    auto base_time = std::chrono::steady_clock::now().time_since_epoch();

    // Create a realistic trade stream with various price movements
    std::vector<double> prices = {100.25, 100.50, 99.75, 101.00, 100.00, 102.50, 99.50, 103.25, 98.75, 101.75};

    std::vector<spl::metrics::metrics> results;

    for (size_t i = 0; i < prices.size(); ++i) {
        auto trade  = this->create_trade(prices[i], i + 1, base_time + std::chrono::milliseconds{i * 50});
        auto result = (*this->multimeter_)(trade);
        results.push_back(result);

        // Basic sanity checks
        EXPECT_DOUBLE_EQ(static_cast<double>(result.minimum),
                         *std::min_element(prices.begin(), prices.begin() + i + 1));
        EXPECT_DOUBLE_EQ(static_cast<double>(result.maximum),
                         *std::max_element(prices.begin(), prices.begin() + i + 1));

        // Mean should be sum/count
        double expected_mean = std::accumulate(prices.begin(), prices.begin() + i + 1, 0.0) / (i + 1);
        EXPECT_NEAR(static_cast<double>(result.mean), expected_mean, 1e-10);
    }
}

TYPED_TEST(MultimeterConsistencyTest, DuplicatePricesHandling) {
    auto base_time = std::chrono::steady_clock::now().time_since_epoch();

    // Test with duplicate prices
    std::vector<double> prices = {100.0, 100.0, 200.0, 200.0, 150.0};

    for (size_t i = 0; i < prices.size(); ++i) {
        auto trade  = this->create_trade(prices[i], i + 1, base_time + std::chrono::milliseconds{i * 25});
        auto result = (*this->multimeter_)(trade);

        // Verify basic properties hold
        EXPECT_LE(static_cast<double>(result.minimum), static_cast<double>(result.maximum));
        EXPECT_GE(static_cast<double>(result.median), static_cast<double>(result.minimum));
        EXPECT_LE(static_cast<double>(result.median), static_cast<double>(result.maximum));
    }
}