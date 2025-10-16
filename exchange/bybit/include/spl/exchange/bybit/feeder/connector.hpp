#pragma once

#include "spl/result/result.hpp"
#include "spl/network/common/uri_parser.hpp"
#include "spl/exchange/common/environment.hpp"

#include <string>
#include <tuple>

namespace spl::exchange::bybit::feeder {

    template <spl::exchange::common::environment EnvironmentV>
    struct connector {
        using response_type = std::tuple<std::string, std::string, std::string>;

        template <typename... ArgsT>
        [[nodiscard]] constexpr auto operator()(ArgsT&&... args) noexcept -> spl::result<response_type> {
            auto const url = [&]() -> std::string {
                auto const prefix = []() {
                    if constexpr (EnvironmentV == spl::exchange::common::environment::sandbox) {
                        return "stream-testnet";
                    }
                    return "stream";
                }();
                return std::format("wss://{}.bybit.com/v5/public/spot", prefix);
            }();
            auto const host = std::string(spl::network::common::host(url).value());
            auto const port = "443";
            auto const path = std::string(spl::network::common::path(url).value());
            return std::make_tuple(host, port, path);
        }
    };

} // namespace spl::exchange::bybit::feeder
