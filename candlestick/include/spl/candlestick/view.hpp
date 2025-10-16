#pragma once

#include "spl/protocol/common/price.hpp"
#include "spl/protocol/common/quantity.hpp"

#include <span>

namespace spl::candlestick {

    struct view {
        std::span<spl::protocol::common::price const> open;
        std::span<spl::protocol::common::price const> high;
        std::span<spl::protocol::common::price const> low;
        std::span<spl::protocol::common::price const> close;
        std::span<spl::protocol::common::quantity const> volume;
    };

} // namespace spl::candlestick