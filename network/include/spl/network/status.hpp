#pragma once

#include <cstdint>

namespace spl::network {

    enum class status : std::uint8_t {

        uninitialized,
        connecting,
        reconnecting,
        connected,
        disconnecting,
        disconnected,
        closing,
        closed,

    };

}
