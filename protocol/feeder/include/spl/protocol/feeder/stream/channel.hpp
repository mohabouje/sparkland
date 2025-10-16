#pragma once

#include <cstdint>

namespace spl::protocol::feeder::stream {

    enum class channel : std::uint8_t {

        level1,
        level2,
        level3,
        snapshot,
        trades,
        ticker,
        candlestick,
        unknown,

    };

}
