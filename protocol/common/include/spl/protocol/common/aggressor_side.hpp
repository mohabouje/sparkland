#pragma once

#include <cstdint>

namespace spl::protocol::common {

    enum class aggressor_side : std::uint8_t {

        buy,
        sell,
        none,

    };

}
