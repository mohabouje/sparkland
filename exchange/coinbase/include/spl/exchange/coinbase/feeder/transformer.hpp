#pragma once

#include "spl/meta/typeinfo.hpp"
#include "spl/result/result.hpp"

#include "spl/protocol/feeder/trade/trade_summary.hpp"
#include "spl/protocol/feeder/stream/heartbeat.hpp"
#include "spl/protocol/feeder/stream/ping.hpp"
#include "spl/protocol/feeder/stream/pong.hpp"
#include "spl/protocol/feeder/stream/channel.hpp"
#include "spl/protocol/feeder/stream/subscribe.hpp"
#include "spl/protocol/feeder/stream/subscribed.hpp"
#include "spl/protocol/feeder/stream/unsubscribe.hpp"
#include "spl/protocol/feeder/stream/unsubscribed.hpp"
#include "spl/protocol/feeder/stream/error.hpp"
#include "spl/protocol/coinbase/websocket/public_stream/decoder.hpp"
#include "spl/protocol/coinbase/websocket/public_stream/encoder.hpp"
#include "spl/types/price.hpp"
#include "spl/types/quantity.hpp"

#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <limits>
#include <string>
#include <iterator>
#include <any>
#include <cstdint>
#include <chrono>
#include <sstream>
#include <string_view>

namespace spl::exchange::coinbase::feeder {

    struct transformer {
        [[nodiscard]] static auto parse_iso8601(std::string_view s) noexcept -> std::chrono::nanoseconds {
            std::chrono::sys_time<std::chrono::nanoseconds> tp{};
            std::istringstream is{std::string{s}};
            is >> std::chrono::parse("%Y-%m-%dT%H:%M:%S.%fZ", tp);
            if (is.fail()) [[unlikely]] {
                is.clear();
                is.str(std::string{s});
                is >> std::chrono::parse("%Y-%m-%dT%H:%M:%SZ", tp);
            }

            return is.fail() ? std::chrono::nanoseconds{0} : tp.time_since_epoch();
        }

        [[nodiscard]] constexpr auto to_channel(spl::protocol::feeder::stream::channel type,
                                                std::string_view symbol) noexcept -> std::string {
            switch (type) {
                case spl::protocol::feeder::stream::channel::trades:
                    return "ticker";
                default:
                    return "not supported";
            }
        }

        template <typename FunctorT>
        [[nodiscard]] auto operator()(spl::protocol::feeder::stream::heartbeat const& input,
                                      FunctorT&& functor) noexcept -> result<void> {
            return spl::success();
        }

        template <typename FunctorT>
        [[nodiscard]] auto operator()(spl::protocol::feeder::stream::ping const& input, FunctorT&& functor) noexcept
            -> result<void> {
            return spl::success();
        }

        template <typename FunctorT>
        [[nodiscard]] auto operator()(spl::protocol::feeder::stream::pong const& input, FunctorT&& functor) noexcept
            -> result<void> {
            return spl::success();
        }

        template <typename FunctorT>
        [[nodiscard]] auto operator()(spl::protocol::coinbase::websocket::public_stream::heartbeat const& heartbeat,
                                      FunctorT&& functor) noexcept -> spl::result<void> {
            return functor(spl::protocol::feeder::stream::heartbeat{
                .exchange_id = spl::protocol::common::exchange_id::coinbase,
                .timestamp   = std::chrono::steady_clock::now().time_since_epoch(),
            });
        }

        template <typename FunctorT>
        [[nodiscard]] auto
            operator()(spl::protocol::coinbase::websocket::public_stream::subscribe::response const& input,
                       FunctorT&& functor) noexcept -> spl::result<void> {
            return functor(spl::protocol::feeder::stream::subscribed{
                .exchange_id = spl::protocol::common::exchange_id::coinbase,
                .channel     = spl::protocol::feeder::stream::channel::trades,
                .timestamp   = std::chrono::steady_clock::now().time_since_epoch(),
            });
        }

        template <typename FunctorT>
        [[nodiscard]] auto operator()(spl::protocol::coinbase::websocket::public_stream::ticker::ticker const& input,
                                      FunctorT&& functor) noexcept -> spl::result<void> {
            auto const price       = spl::types::price::from(input.price);
            auto const quantity    = spl::types::quantity::from(input.last_size);
            auto const side        = spl::protocol::common::aggressor_side(input.side == "buy");
            auto const sequence    = spl::protocol::common::sequence(input.sequence);
            auto const nanoseconds = parse_iso8601(input.time);

            return functor(spl::protocol::feeder::trade::trade_summary{
                .exchange_id = spl::protocol::common::exchange_id::coinbase,
                .side        = side,
                .price       = price,
                .quantity    = quantity,
                .sequence    = sequence,
                .timestamp   = spl::protocol::common::timestamp(nanoseconds),
            });
        }

        template <typename FunctorT>
        [[nodiscard]] auto operator()(spl::protocol::feeder::stream::subscribe const& subscribe,
                                      FunctorT&& functor) noexcept -> spl::result<void> {
            using namespace spl::protocol::coinbase::websocket::public_stream;

            return std::forward<FunctorT>(functor)(subscribe::request{
                .channels = {subscribe::channel{
                    .name        = to_channel(subscribe.channel, subscribe.instrument_id),
                    .product_ids = {std::string(subscribe.instrument_id)},
                }},
            });
        }

        template <typename FunctorT>
        [[nodiscard]] auto operator()(spl::protocol::feeder::stream::unsubscribe const& unsubscribe,
                                      FunctorT&& functor) noexcept -> spl::result<void> {
            using namespace spl::protocol::coinbase::websocket::public_stream;

            return std::forward<FunctorT>(functor)(unsubscribe::request{
                .channels = {subscribe::channel{
                    .name        = to_channel(unsubscribe.channel, unsubscribe.instrument_id),
                    .product_ids = {std::string(unsubscribe.instrument_id)},
                }},
            });
        }

        template <typename RandomT, typename FunctorT>
        [[nodiscard]] constexpr auto operator()(RandomT const& snapshot, FunctorT&& functor) noexcept {
            return std::invoke(std::forward<FunctorT>(functor), snapshot);
        }
    };

} // namespace spl::exchange::coinbase::feeder
