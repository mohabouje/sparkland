#pragma once

#include "spl/exchange/factory/feeder.hpp"
#include "spl/candlestick/candlestick.hpp"

namespace spl::fetcher {

    template <spl::protocol::common::exchange_id ExchangeV, //
              spl::candlestick::type TypeV>
    struct fetcher {
        using feeder_type                 = spl::exchange::factory::feeder<ExchangeV>;
        using contract_type               = typename feeder_type::contract_type;
        using generator_type              = spl::candlestick::generator<TypeV>;
        constexpr static auto exchange_id = ExchangeV;
        constexpr static auto type        = TypeV;

        template <typename... ArgsT>
        constexpr fetcher(spl::network::context& context,                        //
                          spl::components::feeder::session_id const& session_id, //
                          ArgsT&&... args) :
            feeder_(context, session_id), generator_(std::forward<ArgsT>(args)...) {}

        [[nodiscard]] constexpr auto connect() noexcept -> spl::result<void> {
            return feeder_.connect();
        }

        [[nodiscard]] constexpr auto configure(std::chrono::nanoseconds heartbeat,
                                               std::chrono::nanoseconds ping) noexcept -> bool {
            return feeder_.configure(heartbeat, ping);
        }

        [[nodiscard]] constexpr auto session() noexcept -> feeder_type& {
            return feeder_;
        }

        [[nodiscard]] constexpr auto generator() noexcept -> generator_type& {
            return generator_;
        }

        [[nodiscard]] constexpr auto session() const noexcept -> feeder_type const& {
            return feeder_;
        }

        [[nodiscard]] constexpr auto generator() const noexcept -> generator_type const& {
            return generator_;
        }

        [[nodiscard]] constexpr auto operator()(spl::protocol::feeder::stream::subscribe const& subscribe) noexcept
            -> result<void> {
            return feeder_.send(subscribe);
        }

        [[nodiscard]] constexpr auto operator()(spl::protocol::feeder::stream::unsubscribe const& unsubscribe) noexcept
            -> result<void> {
            return feeder_.send(unsubscribe);
        }

        template <typename HandlerT>
        [[nodiscard]] constexpr auto poll(HandlerT&& handler) noexcept -> result<void> {
            return feeder_.poll([this, &handler]<typename EventT>(EventT&& event) -> result<void> {
                logger::info("<= {}", std::forward<EventT>(event));
                if constexpr (requires { generator_(std::forward<EventT>(event), handler); }) {
                    return generator_(std::forward<EventT>(event), handler);
                }
                return spl::success();
            });
        }

    private:
        feeder_type feeder_;
        generator_type generator_;
    };

} // namespace spl::fetcher