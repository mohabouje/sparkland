#pragma once

#include "spl/protocol/coinbase/websocket/public_stream/heartbeat.hpp"
#include "spl/protocol/coinbase/websocket/public_stream/subscribe/response.hpp"
#include "spl/protocol/coinbase/websocket/public_stream/ticker/ticker.hpp"

#include <spl/logger/logger.hpp>
#include <spl/reflect/reflect.hpp>
#include <spl/result/result.hpp>

#include <string_view>

#include <frozen/unordered_map.h>
#include <frozen/string.h>

namespace spl::protocol::coinbase::websocket::public_stream {

    template <typename DecoderT, template <typename ObjectT> class TaggerT>
    struct decoder {
        using decoder_type = std::decay_t<DecoderT>;
        using value_type   = std::variant<coinbase::websocket::public_stream::heartbeat,             //
                                          coinbase::websocket::public_stream::subscribe::response,   //
                                          coinbase::websocket::public_stream::ticker::ticker>;
        using tagger_type  = TaggerT<value_type>;
        using error_type   = typename decoder_type::error_type;

        struct hasher {
            constexpr static auto mapper = frozen::unordered_map<frozen::string, std::size_t, 3>{
                {spl::reflect::identifier<heartbeat>::unique_id,              0},
                {spl::reflect::identifier<subscribe::response>::unique_id,    1},
                {spl::reflect::identifier<ticker::ticker>::unique_id,         2}
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

        template <typename DecoderInstanceT, typename TaggerInstanceT>
        constexpr decoder(DecoderInstanceT&& decoder, TaggerInstanceT&& tagger) noexcept :
            decoder_(std::forward<DecoderInstanceT>(decoder)), tagger_(std::forward<TaggerInstanceT>(tagger)) {}

        constexpr decoder() = default;

        template <typename BufferT, typename HandlerT>
        [[nodiscard, gnu::hot]] constexpr auto operator()(BufferT&& buffer, HandlerT&& handler) const noexcept
            -> error_type {
            auto const [candidate, header] = tagger_.template hash<'t', 'y', 'p', 'e'>(buffer);
            auto const view                = buffer.subspan(header);
            switch (hasher::hash(candidate)) {
                case hasher::template hash<spl::protocol::coinbase::websocket::public_stream::heartbeat>(): {
                    using object_type  = spl::protocol::coinbase::websocket::public_stream::heartbeat;
                    auto const bytes   = err_return(decode<object_type>(view, std::forward<HandlerT>(handler)));
                    auto const decoded = header + bytes;
                    return decoded;
                }
                case hasher::template hash<spl::protocol::coinbase::websocket::public_stream::subscribe::response>(): {
                    using object_type  = spl::protocol::coinbase::websocket::public_stream::subscribe::response;
                    auto const bytes   = err_return(decode<object_type>(view, std::forward<HandlerT>(handler)));
                    auto const decoded = header + bytes;
                    return decoded;
                }
                case hasher::template hash<spl::protocol::coinbase::websocket::public_stream::ticker::ticker>(): {
                    using object_type  = spl::protocol::coinbase::websocket::public_stream::ticker::ticker;
                    auto const bytes   = err_return(decode<object_type>(view, std::forward<HandlerT>(handler)));
                    auto const decoded = header + bytes;
                    return decoded;
                }
                default: {
                    using object_type = std::decay_t<decltype(*this)>;
                    auto const* data  = reinterpret_cast<char const*>(std::data(buffer));
                    auto const view   = std::string_view(data, std::size(buffer));
                    spl::logger::warn("{}: tag {} not recognized in {}", spl::meta::type<object_type>(), candidate,
                                      view);
                    return std::size(view);
                }
            }
        }

        template <typename ObjectT, typename BufferT, typename HandlerT>
        [[nodiscard, gnu::hot]] constexpr auto decode(BufferT&& buffer, HandlerT&& handler) const noexcept
            -> error_type {
            return err_return(
                decoder_.template decode<ObjectT>(std::forward<BufferT>(buffer), std::forward<HandlerT>(handler)));
        }

        [[nodiscard]] constexpr auto tagger() const noexcept -> auto const& {
            return tagger_;
        }

        [[nodiscard]] constexpr auto tagger() noexcept -> auto& {
            return tagger_;
        }

    private:
        decoder_type decoder_{};
        tagger_type tagger_{};
    };

} // namespace spl::protocol::coinbase::websocket::public_stream
