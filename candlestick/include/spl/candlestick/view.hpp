#pragma once

#include <spl/api/feeder/candlestick/candlestick.hpp>

namespace spl::candlestick {

    struct view {
        std::span<spl::protocol::common::price const> open;
        std::span<spl::protocol::common::price const> high;
        std::span<spl::protocol::common::price const> low;
        std::span<spl::protocol::common::price const> close;
        std::span<spl::protocol::common::quantity const> volume;
    };

} // namespace spl::candlestick