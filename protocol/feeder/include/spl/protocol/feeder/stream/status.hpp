#pragma once

#include <cstdint>

namespace spl::protocol::feeder::stream {

    enum class status : std::uint8_t {

        closed,
        trading,
        halted,
        auction,

    };

}
