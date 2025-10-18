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

#include <benchmark/benchmark.h>
#include <vector>
#include <chrono>
#include <random>

using trade_summary    = spl::protocol::feeder::trade::trade_summary;
using ScanMultimeter   = spl::metrics::scan::multimeter<trade_summary>;
using StreamMultimeter = spl::metrics::stream::multimeter<trade_summary>;

// Configuration
namespace {
    constexpr std::size_t FIXED_TRADES = 20000;
    constexpr std::uint32_t SEED       = 42;
} // namespace

// Trade generator
static std::vector<trade_summary> generate_trades(double events_per_second) {
    auto trades = std::vector<trade_summary>{};
    trades.reserve(FIXED_TRADES);

    auto rng        = std::mt19937{SEED};
    auto price_dist = std::uniform_real_distribution<double>{95.0, 105.0};
    auto side_dist  = std::uniform_int_distribution<int>{0, 1};

    auto const avg_interval_ns = static_cast<std::int64_t>(1'000'000'000.0 / events_per_second);
    auto const min_interval_ns = static_cast<std::int64_t>(avg_interval_ns * 0.8);
    auto const max_interval_ns = static_cast<std::int64_t>(avg_interval_ns * 1.2);
    auto interval_dist         = std::uniform_int_distribution<std::int64_t>{min_interval_ns, max_interval_ns};

    auto current_time = std::chrono::steady_clock::now().time_since_epoch();

    for (std::size_t i = 0; i < FIXED_TRADES; ++i) {
        auto const price       = price_dist(rng);
        auto const interval_ns = interval_dist(rng);
        current_time += std::chrono::nanoseconds{interval_ns};

        auto const side = side_dist(rng) == 0 ? spl::protocol::common::aggressor_side::buy
                                              : spl::protocol::common::aggressor_side::sell;

        trades.emplace_back(trade_summary{.instrument_id = spl::protocol::common::instrument_id{1},
                                          .exchange_id   = spl::protocol::common::exchange_id::bybit,
                                          .trade_id      = spl::protocol::common::trade_id{static_cast<char>(i)},
                                          .side          = side,
                                          .price         = spl::protocol::common::price::from(price),
                                          .quantity      = spl::protocol::common::quantity::from(1.0),
                                          .condition     = spl::protocol::common::trade_condition{},
                                          .sequence      = spl::protocol::common::sequence{i},
                                          .timestamp     = spl::protocol::common::timestamp{current_time}});
    }

    return trades;
}

// Scan multimeter benchmark
static void BM_ScanMultimeter(benchmark::State& state) {
    auto const event_rate      = static_cast<double>(state.range(0));
    auto const window_duration = std::chrono::seconds{static_cast<int>(state.range(1))};
    auto const trades          = generate_trades(event_rate);

    for (auto _ : state) {
        auto multimeter = ScanMultimeter{window_duration};

        for (auto const& trade : trades) {
            auto result = multimeter(trade);
            benchmark::DoNotOptimize(result);
        }
    }

    state.SetItemsProcessed(state.iterations() * FIXED_TRADES);
    state.counters["events_per_sec"]  = event_rate;
    state.counters["window_size_sec"] = state.range(1);
}

// Stream multimeter benchmark
static void BM_StreamMultimeter(benchmark::State& state) {
    auto const event_rate      = static_cast<double>(state.range(0));
    auto const window_duration = std::chrono::seconds{static_cast<int>(state.range(1))};
    auto const trades          = generate_trades(event_rate);

    for (auto _ : state) {
        auto multimeter = StreamMultimeter{window_duration};

        for (auto const& trade : trades) {
            auto result = multimeter(trade);
            benchmark::DoNotOptimize(result);
        }
    }

    state.SetItemsProcessed(state.iterations() * FIXED_TRADES);
    state.counters["events_per_sec"]  = event_rate;
    state.counters["window_size_sec"] = state.range(1);
}

// Benchmark registrations: Args(event_rate, window_seconds)
BENCHMARK(BM_ScanMultimeter)
    ->Args({1, 1})
    ->Args({1, 10})
    ->Args({1, 60})
    ->Args({1, 180})
    ->Args({1, 300})
    ->Args({10, 1})
    ->Args({10, 10})
    ->Args({10, 60})
    ->Args({10, 180})
    ->Args({10, 300})
    ->Args({1000, 1})
    ->Args({1000, 10})
    ->Args({1000, 60})
    ->Args({1000, 180})
    ->Args({1000, 300})
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_StreamMultimeter)
    ->Args({1, 1})
    ->Args({1, 10})
    ->Args({1, 60})
    ->Args({1, 180})
    ->Args({1, 300})
    ->Args({10, 1})
    ->Args({10, 10})
    ->Args({10, 60})
    ->Args({10, 180})
    ->Args({10, 300})
    ->Args({1000, 1})
    ->Args({1000, 10})
    ->Args({1000, 60})
    ->Args({1000, 180})
    ->Args({1000, 300})
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
