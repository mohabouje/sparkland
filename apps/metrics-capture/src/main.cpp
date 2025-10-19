#include "spl/exchange/factory/feeder.hpp"
#include "spl/metrics/multimeter.hpp"
#include "spl/logger/logger.hpp"
#include "spl/protocol/feeder/trade/trade_summary.hpp"
#include "spl/protocol/common/exchange_id.hpp"

#include <CLI/CLI.hpp>
#include <iostream>
#include <fstream>
#include <format>
#include <chrono>
#include <csignal>
#include <atomic>
#include <string>

struct arguments {
    spl::protocol::common::exchange_id exchange_id{spl::protocol::common::exchange_id::coinbase};
    spl::protocol::common::instrument_id instrument_id{"BTC-USDT"};
    spl::protocol::common::timestamp period{std::chrono::minutes(5)};
    spl::protocol::common::timestamp duration{std::chrono::hours(1)};
    spl::metrics::type type{spl::metrics::type::stream};

    [[nodiscard]] static auto from(int argc, char** argv) noexcept -> spl::result<arguments> {
        CLI::App app{"Sparkland Metrics Capture - Real-time exchange metrics collector"};

        auto args           = arguments{};
        auto exchange_str   = std::string(spl::reflect::enum_to_string(args.exchange_id));
        auto metrics_str    = std::string(spl::reflect::enum_to_string(args.type));
        auto instrument_str = std::string(args.instrument_id);
        auto window         = std::chrono::duration_cast<std::chrono::minutes>(args.period).count();
        auto duration       = std::chrono::duration_cast<std::chrono::minutes>(args.duration).count();

        app.add_option("-e,--exchange", exchange_str, "Exchange to connect to (bybit, coinbase)")
            ->default_val("bybit")
            ->check(CLI::IsMember({"bybit", "coinbase"}));

        app.add_option("-m,--metrics", metrics_str, "Metrics type to use (stream, scan)")
            ->default_val("stream")
            ->check(CLI::IsMember({"stream", "scan"}));

        app.add_option("-i,--instrument", instrument_str, "Instrument to track (e.g., BTCUSDT)")
            ->default_val("BTC-USDT");

        app.add_option("-w,--window", window, "Window size in minutes for metrics calculation")
            ->default_val(5)
            ->check(CLI::PositiveNumber);

        app.add_option("-d,--duration", duration, "Duration in minutes to run")
            ->default_val(60)
            ->check(CLI::PositiveNumber);

        try {
            app.parse(argc, argv);
            return spl::success();
        } catch (const CLI::ParseError& e) {
            std::ignore = app.exit(e);
            return spl::failure("Failed to parse command-line arguments: {}", e.what());
        }

        args.exchange_id   = spl::reflect::enum_from_string<spl::protocol::common::exchange_id>(exchange_str);
        args.type          = spl::reflect::enum_from_string<spl::metrics::type>(metrics_str);
        args.instrument_id = spl::protocol::common::instrument_id{instrument_str};
        args.period        = spl::protocol::common::timestamp{std::chrono::minutes(window)};
        args.duration      = spl::protocol::common::timestamp{std::chrono::minutes(duration)};
        return args;
    }
};

template <spl::protocol::common::exchange_id ExchangeIdV, spl::metrics::type MetricsTypeV,
          spl::exchange::common::environment EnvironmentV = spl::exchange::common::environment::production>
[[nodiscard]] constexpr auto execute(arguments const& args) -> spl::result<void> {
    using session_type    = spl::exchange::factory::feeder<ExchangeIdV, EnvironmentV>;
    using trade_summary   = spl::protocol::feeder::trade::trade_summary;
    using multimeter_type = spl::metrics::multimeter<MetricsTypeV, trade_summary>;

    auto context    = spl::network::context();
    auto identifier = spl::components::feeder::session_id{"metrics-capture", "exchange"};
    auto session    = session_type(context, identifier);
    auto multimeter = multimeter_type(std::chrono::minutes(5));

    spl::logger::info("Connecting to exchange...");
    err_return(session.connect());

    spl::logger::info("Subscribing to {} trades...", args.instrument_id);
    err_return(session.send(spl::protocol::feeder::stream::subscribe{
        .exchange_id   = ExchangeIdV,
        .instrument_id = args.instrument_id,
        .channel       = spl::protocol::feeder::stream::channel::trades,
    }));

    spl::logger::info("Starting to capture metrics...");
    auto const current_time = std::chrono::system_clock::now();
    auto const end_time     = current_time + args.duration;
    while (std::chrono::system_clock::now() < end_time) {
        err_return(session.poll([&]<typename EventT>(EventT&& event) -> spl::result<void> {
            if constexpr (requires { multimeter(std::forward<EventT>(event)); }) {
                auto const metrics = multimeter(std::forward<EventT>(event));
                spl::logger::info("{}", metrics);
            }
            return spl::success();
        }));
    }

    return spl::success();
}

template <spl::metrics::type MetricsTypeV>
[[nodiscard]] constexpr auto execute(arguments const& args) -> spl::result<void> {
    switch (args.exchange_id) {
        case spl::protocol::common::exchange_id::bybit:
            return execute<spl::protocol::common::exchange_id::bybit, MetricsTypeV>(args);
        case spl::protocol::common::exchange_id::coinbase:
            return execute<spl::protocol::common::exchange_id::coinbase, MetricsTypeV>(args);
        default:
            return spl::failure("Unsupported exchange ID");
    }
}

[[nodiscard]] constexpr auto execute(arguments const& args) -> spl::result<void> {
    switch (args.type) {
        case spl::metrics::type::stream:
            return execute<spl::metrics::type::stream>(args);
        case spl::metrics::type::scan:
            return execute<spl::metrics::type::scan>(args);
        default:
            return spl::failure("Unsupported metrics type");
    }
}

[[nodiscard]] constexpr auto execute(int argc, char** argv) -> spl::result<void> {
    auto const args = arguments::from(argc, argv);
    if (spl::failed(args)) {
        return spl::success();
    }
    return execute(args.value());
}

auto main(int argc, char** argv) -> int {
    if (auto const result = execute(argc, argv); spl::failed(result)) {
        spl::logger::error("Application error: {}", result.error().message().data());
        return -1;
    }
    return 0;
}
