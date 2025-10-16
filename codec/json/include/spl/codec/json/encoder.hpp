#pragma once

#include "spl/codec/json/json.hpp"

namespace spl::codec::json {

    struct encoder {
        using error_type = spl::result<std::size_t>;

        template <typename ObjectT, typename BufferT>
        [[nodiscard]] static auto encode(ObjectT&& input, BufferT&& output) noexcept -> error_type {
            return to_json(std::forward<ObjectT>(input), std::forward<BufferT>(output));
        }

        template <typename ObjectT, typename BufferT>
        [[nodiscard]] static auto operator()(ObjectT&& input, BufferT&& output) noexcept -> error_type {
            return encode(std::forward<ObjectT>(input), std::forward<BufferT>(output));
        }
    };

} // namespace spl::codec::json
