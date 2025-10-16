#pragma once

#include "spl/codec/json/json.hpp"

namespace spl::codec::json {

    struct decoder {
        using error_type = spl::result<std::size_t>;

        template <typename BufferT, typename ObjectT>
        [[nodiscard]] static auto decode(BufferT&& input, ObjectT& object) noexcept -> error_type {
            auto const size    = length(std::forward<BufferT>(input));
            auto const subview = std::span{std::data(input), size};
            err_return(spl::codec::json::from_json(subview, object));
            return size;
        }

        template <typename ObjectT, typename BufferT, typename FunctorT>
        [[nodiscard]] static auto decode(BufferT&& input, FunctorT&& functor) noexcept -> error_type {
            auto temporal = ObjectT{};
            auto result   = err_return(decode(std::forward<BufferT>(input), temporal));
            err_return(functor(temporal));
            return result;
        }

        template <typename BufferT, typename ObjectT>
        [[nodiscard]] static auto operator()(BufferT&& input, ObjectT& object) noexcept -> error_type {
            return decode(std::forward<BufferT>(input), object);
        }

        template <typename BufferT>
        [[nodiscard]] constexpr static auto length(BufferT&& input) noexcept -> std::size_t {
            auto const opening_char = input[0];
            auto const closing_char = (opening_char == '{') ? '}' : ']';
            auto const index        = (opening_char == '{') ? 0 : 1;
            auto counter            = std::array{0, 0};
            for (std::size_t i = 0; i < std::size(input); ++i) {
                auto const ch = input[i];
                counter[index] += (ch == opening_char) - (ch == closing_char);
                if (counter[index] == 0) {
                    return i + 1;
                }
            }
            return 0;
        }
    };

} // namespace spl::codec::json
