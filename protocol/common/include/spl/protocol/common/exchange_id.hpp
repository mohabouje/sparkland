#pragma once

#include <cstdint>

namespace spl::protocol::common {

    enum class exchange_id : std::uint8_t {

        unknown,
        binance,
        bybit,
        kucoin,
        poloniex

    };
}
