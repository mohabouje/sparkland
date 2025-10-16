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
    struct generator<spl::candlestick::type::tick> {
        using value_type = spl::protocol::feeder::candlestick::candlestick;

        constexpr generator(std::size_t threshold) noexcept : threshold_{threshold} {}

        template <typename FunctorT>
        [[nodiscard]] constexpr auto operator()(spl::protocol::feeder::trade::trade_summary const& object,
                                                FunctorT&& functor) noexcept -> spl::result<void> {
            auto const accumulating = current_.has_value();
            auto const previous_qt  = accumulating ? current_->volume : spl::protocol::common::quantity::zero();
            if (not accumulating) [[unlikely]] {
                err_return(reset(object));
            }

            accumulated_        = accumulating ? accumulated_ + 1 : accumulated_;
            current_->high      = std::max(current_->high, object.price);
            current_->low       = std::min(current_->low, object.price);
            current_->close     = object.price;
            current_->ending    = object.timestamp;
            current_->volume    = object.quantity + previous_qt;
            current_->trades    = current_->trades + 1;
            current_->timestamp = spl::protocol::common::timestamp(std::chrono::system_clock::now().time_since_epoch());

            if (accumulated_ == threshold_) [[unlikely]] {
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
                .ending        = object.timestamp,
                .sequence      = ++sequence_,
                .timestamp     = spl::protocol::common::timestamp(std::chrono::system_clock::now().time_since_epoch()),
            }));
            current_ = std::nullopt;
            return spl::success();
        }

        [[nodiscard]] constexpr auto get() noexcept -> std::optional<value_type> const& {
            return current_;
        }

    private:
        [[nodiscard]] constexpr auto reset(spl::protocol::feeder::trade::trade_summary const& object) noexcept
            -> result<void> {
            accumulated_ = 1;
            current_     = spl::protocol::feeder::candlestick::candlestick{
                    .instrument_id = object.instrument_id,
                    .exchange_id   = object.exchange_id,
                    .open          = object.price,
                    .high          = object.price,
                    .low           = object.price,
                    .close         = object.price,
                    .volume        = object.quantity,
                    .trades        = spl::protocol::common::counter{0},
                    .complete      = false,
                    .starting      = object.timestamp,
                    .ending        = object.timestamp,
                    .timestamp     = object.timestamp,
            };
            return spl::success();
        }

        std::size_t threshold_{};
        std::size_t accumulated_{};
        spl::protocol::common::sequence sequence_{0};
        std::optional<value_type> current_{std::nullopt};
    };

} // namespace spl::candlestick