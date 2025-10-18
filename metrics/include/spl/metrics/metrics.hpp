#pragma once

#include "spl/types/price.hpp"

#include <chrono>

namespace spl::metrics {

    struct metrics {
        spl::types::price minimum;
        spl::types::price maximum;
        spl::types::price median;
        spl::types::price mean;
        std::chrono::nanoseconds timestamp;
    };

} // namespace spl::metrics