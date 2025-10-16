#pragma once

#include "spl/meta/typeinfo.hpp"
#include "spl/result/result.hpp"

#include "spl/protocol/feeder/stream/heartbeat.hpp"
#include "spl/protocol/feeder/stream/ping.hpp"
#include "spl/protocol/feeder/stream/pong.hpp"
#include "spl/protocol/feeder/stream/channel.hpp"
#include "spl/protocol/feeder/stream/subscribe.hpp"
#include "spl/protocol/feeder/stream/subscribed.hpp"
#include "spl/protocol/feeder/stream/unsubscribe.hpp"
#include "spl/protocol/feeder/stream/unsubscribed.hpp"
#include "spl/protocol/feeder/stream/error.hpp"
#include "spl/protocol/bybit/websocket/public_stream/decoder.hpp"
#include "spl/protocol/bybit/websocket/public_stream/encoder.hpp"
#include "spl/types/price.hpp"
#include "spl/types/quantity.hpp"

#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <chrono>
#include <limits>
#include <string>
#include <iterator>
#include <any>
#include <cstdint>

namespace spl::exchange::bybit::feeder {

    struct transformer {
        [[nodiscard]] constexpr auto to_channel(spl::protocol::feeder::stream::channel type,
                                                std::string_view symbol) noexcept -> std::string {
            switch (type) {
                case spl::protocol::feeder::stream::channel::level2:
                    return std::format("orderbook.{}.{}", 50, symbol);
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
            return functor(spl::protocol::bybit::websocket::public_stream::ping{});
        }

        template <typename FunctorT>
        [[nodiscard]] auto operator()(spl::protocol::feeder::stream::pong const& input, FunctorT&& functor) noexcept
            -> result<void> {
            return functor(spl::protocol::bybit::websocket::public_stream::pong{});
        }

        template <typename FunctorT>
        [[nodiscard]] auto operator()(spl::protocol::bybit::websocket::public_stream::ping const& ping,
                                      FunctorT&& functor) noexcept -> spl::result<void> {
            if (ping.ret_msg == "pong") [[unlikely]] {
                return functor(spl::protocol::feeder::stream::pong{
                    .exchange_id = spl::protocol::common::exchange_id::bybit,
                    .timestamp   = std::chrono::steady_clock::now().time_since_epoch(),
                });
            }
            return functor(spl::protocol::feeder::stream::ping{
                .exchange_id = spl::protocol::common::exchange_id::bybit,
                .timestamp   = std::chrono::steady_clock::now().time_since_epoch(),
            });
        }

        template <typename FunctorT>
        [[nodiscard]] auto operator()(spl::protocol::bybit::websocket::public_stream::pong const& pong,
                                      FunctorT&& functor) noexcept -> spl::result<void> {
            return functor(spl::protocol::feeder::stream::pong{
                .exchange_id = spl::protocol::common::exchange_id::bybit,
                .timestamp   = std::chrono::steady_clock::now().time_since_epoch(),
            });
        }

        template <typename FunctorT>
        [[nodiscard]] auto operator()(spl::protocol::bybit::websocket::public_stream::subscribe::response const& input,
                                      FunctorT&& functor) noexcept -> spl::result<void> {
            if (not input.success) {
                return functor(spl::protocol::feeder::stream::error{
                    .exchange_id = spl::protocol::common::exchange_id::bybit,
                    .code        = spl::protocol::common::error_code{},
                    .message     = spl::protocol::common::error_message(input.ret_msg),
                    .timestamp   = std::chrono::steady_clock::now().time_since_epoch(),
                });
            }

            return functor(spl::protocol::feeder::stream::subscribed{
                .exchange_id = spl::protocol::common::exchange_id::bybit,
                .channel     = spl::protocol::feeder::stream::channel::level2,
                .timestamp   = std::chrono::steady_clock::now().time_since_epoch(),
            });
        }

        template <typename FunctorT>
        [[nodiscard]] auto
            operator()(spl::protocol::bybit::websocket::public_stream::unsubscribe::response const& input,
                       FunctorT&& functor) noexcept -> spl::result<void> {
            if (not input.success) [[unlikely]] {
                return functor(spl::protocol::feeder::stream::error{
                    .exchange_id = spl::protocol::common::exchange_id::bybit,
                    .code        = spl::protocol::common::error_code{},
                    .message     = spl::protocol::common::error_message(input.ret_msg),
                    .timestamp   = std::chrono::steady_clock::now().time_since_epoch(),
                });
            }

            return functor(spl::protocol::feeder::stream::unsubscribed{
                .exchange_id = spl::protocol::common::exchange_id::bybit,
                .channel     = spl::protocol::feeder::stream::channel::level2,
                .timestamp   = std::chrono::steady_clock::now().time_since_epoch(),
            });
        }

        template <typename FunctorT>
        [[nodiscard]] auto operator()(spl::protocol::feeder::stream::subscribe const& subscribe,
                                      FunctorT&& functor) noexcept -> spl::result<void> {
            return std::forward<FunctorT>(functor)(spl::protocol::bybit::websocket::public_stream::subscribe::request{
                .req_id = std::to_string(std::rand()),
                .args   = {to_channel(subscribe.channel, subscribe.instrument_id)},
            });
        }

        template <typename FunctorT>
        [[nodiscard]] auto operator()(spl::protocol::feeder::stream::unsubscribe const& unsubscribe,
                                      FunctorT&& functor) noexcept -> spl::result<void> {
            return std::forward<FunctorT>(functor)(spl::protocol::bybit::websocket::public_stream::unsubscribe::request{
                .req_id = std::to_string(std::rand()),
                .args   = {to_channel(unsubscribe.channel, unsubscribe.instrument_id)},
            });
        }

        template <typename RandomT, typename FunctorT>
        [[nodiscard]] constexpr auto operator()(RandomT const& snapshot, FunctorT&& functor) noexcept {
            return std::invoke(std::forward<FunctorT>(functor), snapshot);
        }
    };

} // namespace spl::exchange::bybit::feeder