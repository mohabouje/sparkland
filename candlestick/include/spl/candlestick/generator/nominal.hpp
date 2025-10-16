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
    struct generator<spl::candlestick::type::nominal> {
        using value_type = spl::protocol::feeder::candlestick::candlestick;

        constexpr generator(spl::protocol::common::price threshold) noexcept : threshold_{threshold} {}

        template <typename FunctorT>
        [[nodiscard]] constexpr auto operator()(spl::protocol::feeder::trade::trade_summary const& object,
                                                FunctorT&& functor) noexcept -> spl::result<void> {
            auto const accumulating = current_.has_value();
            auto const nominal      = object.price * object.quantity.template unshifted<double>();
            auto const previous_qt  = accumulating ? current_->volume : spl::protocol::common::quantity::zero();
            auto const previous_nm  = accumulating ? accumulated_ : spl::protocol::common::price::zero();
            if (not accumulating) [[unlikely]] {
                err_return(reset(object));
            }

            accumulated_        = nominal + previous_nm;
            current_->high      = std::max(current_->high, object.price);
            current_->low       = std::min(current_->low, object.price);
            current_->close     = object.price;
            current_->ending    = object.timestamp;
            current_->volume    = object.quantity + previous_qt;
            current_->trades    = current_->trades + 1;
            current_->timestamp = spl::protocol::common::timestamp(std::chrono::system_clock::now().time_since_epoch());

            if (math::definitely_lt(accumulated_, threshold_)) [[unlikely]] {
                return spl::success();
            }

            auto const required_nm = threshold_ - previous_nm;
            auto const required_cv = required_nm / object.price;
            auto const required_qt = spl::protocol::common::quantity{required_cv.template unshifted<double>()};
            err_return(functor(spl::protocol::feeder::candlestick::candlestick{
                .instrument_id = current_->instrument_id,
                .exchange_id   = current_->exchange_id,
                .open          = current_->open,
                .high          = current_->high,
                .low           = current_->low,
                .close         = current_->close,
                .volume        = previous_qt + required_qt,
                .trades        = current_->trades,
                .complete      = true,
                .starting      = current_->starting,
                .ending        = object.timestamp,
                .sequence      = ++sequence_,
                .timestamp     = spl::protocol::common::timestamp(std::chrono::system_clock::now().time_since_epoch()),
            }));

            current_                = std::nullopt;
            auto const remaining_nm = nominal - required_nm;
            auto const remaining_cv = remaining_nm / object.price;
            auto const remaining_qt = spl::protocol::common::quantity{remaining_cv.template unshifted<double>()};
            if (math::definitely_let(remaining_qt, spl::protocol::common::quantity::zero())) [[unlikely]] {
                return spl::success();
            }

            return operator()(
                spl::protocol::feeder::trade::trade_summary{
                    .instrument_id = object.instrument_id,
                    .exchange_id   = object.exchange_id,
                    .price         = object.price,
                    .quantity      = std::max(spl::protocol::common::quantity::zero(), remaining_qt),
                    .timestamp     = object.timestamp,
                },
                std::forward<FunctorT>(functor));
        }

        [[nodiscard]] constexpr auto get() noexcept -> std::optional<value_type> const& {
            return current_;
        }

    private:
        [[nodiscard]] constexpr auto reset(spl::protocol::feeder::trade::trade_summary const& object) noexcept
            -> result<void> {
            accumulated_ = object.price * object.quantity.template unshifted<double>();
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

        spl::protocol::common::price threshold_{};
        spl::protocol::common::price accumulated_{};
        spl::protocol::common::sequence sequence_{0};
        std::optional<value_type> current_{std::nullopt};
    };

} // namespace spl::candlestick