#pragma once

#include <cstdint>

namespace spl::protocol::common {

    enum class trade_condition : std::uint8_t {

        regular,
        block,
        retail,

    };

}
