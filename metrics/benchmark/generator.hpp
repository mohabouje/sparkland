#pragma once

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

#include <vector>
#include <random>
#include <chrono>

namespace spl::metrics::benchmark {

    class trade_generator {
    public:
        struct config {
            std::uint32_t seed       = 42;
            std::size_t count        = 20000;
            double min_price         = 90.0;
            double max_price         = 110.0;
            double events_per_second = 100.0; // Target event rate
            std::chrono::milliseconds min_time_increment{0};
            std::chrono::milliseconds max_time_increment{500};
            spl::protocol::common::exchange_id exchange = spl::protocol::common::exchange_id::bybit;
            spl::protocol::common::instrument_id instrument{1};
        };

        explicit trade_generator(config const& cfg) : config_{cfg}, rng_{cfg.seed} {}

        [[nodiscard]] auto generate() -> std::vector<spl::protocol::feeder::trade::trade_summary> {
            auto trades = std::vector<spl::protocol::feeder::trade::trade_summary>{};
            trades.reserve(config_.count);

            auto price_dist = std::uniform_real_distribution<double>{config_.min_price, config_.max_price};
            auto side_dist  = std::uniform_int_distribution<int>{0, 1};

            // Calculate average interval based on events per second
            auto const avg_interval_ns = static_cast<std::int64_t>(1'000'000'000.0 / config_.events_per_second);

            // Add some variance around the average interval (±20%)
            auto const min_interval_ns = static_cast<std::int64_t>(avg_interval_ns * 0.8);
            auto const max_interval_ns = static_cast<std::int64_t>(avg_interval_ns * 1.2);
            auto interval_dist         = std::uniform_int_distribution<std::int64_t>{min_interval_ns, max_interval_ns};

            auto current_time = std::chrono::steady_clock::now().time_since_epoch();

            for (std::size_t i = 0; i < config_.count; ++i) {
                auto const price       = price_dist(rng_);
                auto const interval_ns = interval_dist(rng_);
                current_time += std::chrono::nanoseconds{interval_ns};

                auto const side = side_dist(rng_) == 0 ? spl::protocol::common::aggressor_side::buy
                                                       : spl::protocol::common::aggressor_side::sell;

                trades.emplace_back(spl::protocol::feeder::trade::trade_summary{
                    .instrument_id = config_.instrument,
                    .exchange_id   = config_.exchange,
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

        [[nodiscard]] auto get_config() const noexcept -> config const& {
            return config_;
        }

    private:
        config config_;
        std::mt19937 rng_;
    };

    // Predefined benchmark configurations with different event rates
    namespace configs {
        inline auto low_frequency() -> trade_generator::config {
            return trade_generator::config{.seed              = 42,
                                           .count             = 20000,
                                           .min_price         = 95.0,
                                           .max_price         = 105.0,
                                           .events_per_second = 10.0}; // 10 events/sec
        }

        inline auto medium_frequency() -> trade_generator::config {
            return trade_generator::config{.seed              = 42,
                                           .count             = 20000,
                                           .min_price         = 95.0,
                                           .max_price         = 105.0,
                                           .events_per_second = 100.0}; // 100 events/sec
        }

        inline auto high_frequency() -> trade_generator::config {
            return trade_generator::config{.seed              = 42,
                                           .count             = 20000,
                                           .min_price         = 95.0,
                                           .max_price         = 105.0,
                                           .events_per_second = 1000.0}; // 1000 events/sec
        }

        inline auto ultra_high_frequency() -> trade_generator::config {
            return trade_generator::config{.seed              = 42,
                                           .count             = 20000,
                                           .min_price         = 95.0,
                                           .max_price         = 105.0,
                                           .events_per_second = 10000.0}; // 10k events/sec
        }
    } // namespace configs

    // Window configurations for benchmarking
    namespace windows {
        constexpr auto short_term     = std::chrono::seconds{10};
        constexpr auto medium_term    = std::chrono::seconds{60};
        constexpr auto long_term      = std::chrono::seconds{180};
        constexpr auto very_long_term = std::chrono::seconds{300};
    } // namespace windows

} // namespace spl::metrics::benchmark