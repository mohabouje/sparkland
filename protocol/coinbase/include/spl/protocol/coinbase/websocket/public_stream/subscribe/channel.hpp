#pragma once

#include <spl/reflect/reflect.hpp>

#include <string>
#include <vector>

namespace spl::protocol::coinbase::websocket::public_stream::subscribe {

    struct channel {
        std::string name;
        std::vector<std::string> product_ids;
    };

} // namespace spl::protocol::coinbase::websocket::public_stream::subscribe
