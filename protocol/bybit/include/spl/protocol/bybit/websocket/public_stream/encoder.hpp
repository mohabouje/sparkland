#pragma once

#include "spl/protocol/bybit/websocket/public_stream/ping.hpp"
#include "spl/protocol/bybit/websocket/public_stream/pong.hpp"
#include "spl/protocol/bybit/websocket/public_stream/subscribe/request.hpp"
#include "spl/protocol/bybit/websocket/public_stream/unsubscribe/request.hpp"

#include <spl/reflect/reflect.hpp>
#include <spl/result/result.hpp>

#include <string_view>

#include <frozen/unordered_map.h>
#include <frozen/string.h>

namespace spl::protocol::bybit::websocket::public_stream {

    template <typename EncoderT, template <typename ObjectT> class TaggerT>
    struct encoder {
        using encoder_type = std::decay_t<EncoderT>;
        using value_type   = std::variant<spl::protocol::bybit::websocket::public_stream::subscribe::request,
                                          bybit::websocket::public_stream::unsubscribe::request,
                                          bybit::websocket::public_stream::ping, bybit::websocket::public_stream::pong>;
        using tagger_type  = TaggerT<value_type>;
        using error_type   = typename encoder_type::error_type;

        struct hasher {
            constexpr static auto mapper = frozen::unordered_map<frozen::string, std::size_t, 4>{
                {spl::reflect::identifier<
                     spl::protocol::bybit::websocket::public_stream::subscribe::request>::unique_id,
                 0                                                                                                       },
                {spl::reflect::identifier<
                     spl::protocol::bybit::websocket::public_stream::unsubscribe::request>::unique_id,
                 1                                                                                                       },
                {spl::reflect::identifier<spl::protocol::bybit::websocket::public_stream::ping>::unique_id,             2},
                {spl::reflect::identifier<spl::protocol::bybit::websocket::public_stream::pong>::unique_id,             3},
            };

            template <typename ObjectT>
            [[nodiscard]] constexpr static auto hash() noexcept -> std::uint64_t {
                return mapper.at(spl::reflect::identifier<ObjectT>::unique_id);
            }

            [[nodiscard]] constexpr static auto hash(std::string_view candidate) noexcept -> std::uint64_t {
                return mapper.at(candidate);
            }

            [[nodiscard]] constexpr static auto hash(std::uint64_t candidate) noexcept -> std::uint64_t {
                return candidate;
            }
        };

        template <typename EncoderInstanceT, typename TaggerInstanceT>
        constexpr encoder(EncoderInstanceT&& encoder, TaggerInstanceT&& tagger) noexcept :
            encoder_(std::forward<EncoderInstanceT>(encoder)), tagger_(std::forward<TaggerInstanceT>(tagger)) {}

        constexpr encoder() = default;

        template <typename ObjectT>
        [[nodiscard, gnu::hot]] constexpr auto operator()(ObjectT&& object, std::span<char> buffer) noexcept
            -> error_type {
            using object_type  = std::decay_t<ObjectT>;
            auto const tag     = tagger_type::template outbound<object_type>();
            auto const header  = err_return(tagger_.template header<'o', 'p'>(tag, buffer));
            auto const body    = err_return(encoder_(std::forward<ObjectT>(object), buffer.subspan(header)));
            auto const overall = std::span<char>(std::data(buffer) + header, body);
            auto const footer  = err_return(tagger_.template footer<'o', 'p'>(tag, overall));
            return header + body + footer;
        }

    private:
        encoder_type encoder_{};
        tagger_type tagger_{};
    };

} // namespace spl::protocol::bybit::websocket::public_stream
