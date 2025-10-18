#pragma once

#include "spl/types/price.hpp"
#include "spl/types/timestamp.hpp"

namespace spl::metrics {

    struct metrics {
        spl::types::price minimum;
        spl::types::price maximum;
        spl::types::price median;
        spl::types::price mean;
        spl::types::timestamp timestamp;
    };

} // namespace spl::metrics