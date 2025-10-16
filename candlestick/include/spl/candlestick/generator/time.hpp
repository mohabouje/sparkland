#pragma once

#include "spl/candlestick/generator.hpp"
#include "spl/core/assert.hpp"
#include "spl/math/math.hpp"
#include "spl/result/result.hpp"
#include "spl/logger/logger.hpp"

#include <spl/protocol/feeder/candlestick/candlestick.hpp>
#include <spl/protocol/feeder/trade/trade_summary.hpp>

namespace spl::candlestick {

    template <>
    struct generator<spl::candlestick::type::time> {
        using value_type = spl::protocol::feeder::candlestick::candlestick;

        constexpr generator(std::chrono::nanoseconds threshold) noexcept : threshold_{threshold} {}

        template <typename FunctorT>
        [[nodiscard]] constexpr auto operator()(spl::protocol::feeder::trade::trade_summary const& object,
                                                FunctorT&& functor) noexcept -> spl::result<void> {
            auto const accumulating = current_.has_value();
            auto const previous_qt  = accumulating ? current_->volume : spl::protocol::common::quantity::zero();
            if (not accumulating) [[unlikely]] {
                err_return(reset(object));
            }

            auto const expired = object.timestamp > current_->ending;
            if (not expired) [[likely]] {
                current_->high   = std::max(current_->high, object.price);
                current_->low    = std::min(current_->low, object.price);
                current_->close  = object.price;
                current_->volume = object.quantity + previous_qt;
                current_->trades = current_->trades + 1;
                return spl::success();
            }

            err_return(functor(spl::protocol::feeder::candlestick::candlestick{
                .instrument_id = current_->instrument_id,
                .exchange_id   = current_->exchange_id,
                .open          = current_->open,
                .high          = current_->high,
                .low           = current_->low,
                .close         = current_->close,
                .volume        = current_->volume,
                .trades        = current_->trades,
                .complete      = true,
                .starting      = current_->starting,
                .ending        = current_->ending,
                .sequence      = ++sequence_,
                .timestamp     = spl::protocol::common::timestamp(std::chrono::system_clock::now().time_since_epoch()),
            }));
            current_ = std::nullopt;
            return this->operator()(object, std::forward<FunctorT>(functor));
        }

        [[nodiscard]] constexpr auto get() noexcept -> std::optional<value_type> const& {
            return current_;
        }

    private:
        [[nodiscard]] constexpr auto reset(spl::protocol::feeder::trade::trade_summary const& object) noexcept
            -> result<void> {
            auto const interval_ns = threshold_.count();
            auto const timestamp   = object.timestamp.count();
            auto const starting    = std::chrono::nanoseconds{(timestamp / interval_ns) * interval_ns};
            auto const ending      = starting + threshold_ - std::chrono::nanoseconds(1);

            current_ = spl::protocol::feeder::candlestick::candlestick{
                .instrument_id = object.instrument_id,
                .exchange_id   = object.exchange_id,
                .open          = object.price,
                .high          = object.price,
                .low           = object.price,
                .close         = object.price,
                .volume        = object.quantity,
                .trades        = spl::protocol::common::counter{0},
                .complete      = false,
                .starting      = spl::protocol::common::timestamp(starting),
                .ending        = spl::protocol::common::timestamp(ending),
                .timestamp     = object.timestamp,
            };
            return spl::success();
        }

        std::chrono::nanoseconds threshold_{};
        spl::protocol::common::sequence sequence_{0};
        std::optional<value_type> current_{std::nullopt};
    };

} // namespace spl::candlestick