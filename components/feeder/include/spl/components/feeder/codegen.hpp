#pragma once

#include "spl/concepts/object.hpp"
#include "spl/network/client/wss.hpp"
#include "spl/core/unused.hpp"
#include "spl/result/result.hpp"
#include "spl/components/feeder/session.hpp"
#include "spl/components/feeder/direction.hpp"
#include "spl/protocol/feeder/stream/ping.hpp"
#include "spl/protocol/feeder/stream/pong.hpp"
#include "spl/protocol/feeder/stream/heartbeat.hpp"

#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/make_printable.hpp>

#include <chrono>
#include <string_view>

namespace spl::components::feeder {

    namespace internal {

        template <typename ConnectionT>
        struct traits;

        template <>
        struct traits<spl::network::client::wss> {
            template <spl::components::feeder::direction TypeV>
            using buffer_type = std::conditional_t<TypeV == spl::components::feeder::direction::inbound,
                                                   boost::beast::flat_buffer, std::vector<char>>;
            constexpr static auto serializable = true;
        };

    } // namespace internal

    template <typename TraitT>
    class codegen : public spl::components::feeder::session<TraitT> {
        using base_type = spl::components::feeder::session<TraitT>;

    public:
        using contract_type    = std::decay_t<TraitT>;
        using connection_type  = typename contract_type::connection_type;
        using connector_type   = typename contract_type::connector_type;
        using encoder_type     = typename contract_type::encoder_type;
        using decoder_type     = typename contract_type::decoder_type;
        using transformer_type = typename contract_type::transformer_type;

        template <spl::components::feeder::direction TypeV>
        using buffer_type = typename internal::traits<connection_type>::template buffer_type<TypeV>;

        constexpr codegen(spl::network::context& context, spl::components::feeder::session_id const& session_id,
                          encoder_type encoder = {}, decoder_type decoder = {}) :
            base_type(context, session_id), encoder_(encoder), decoder_(decoder) {}

        [[nodiscard]] constexpr auto encoder() const noexcept -> encoder_type const& {
            return encoder_;
        }

        [[nodiscard]] constexpr auto encoder() noexcept -> encoder_type& {
            return encoder_;
        }

        [[nodiscard]] constexpr auto decoder() const noexcept -> decoder_type const& {
            return decoder_;
        }

        [[nodiscard]] constexpr auto decoder() noexcept -> decoder_type& {
            return decoder_;
        }

        [[nodiscard]] constexpr auto connect() noexcept -> spl::result<void> {
            auto const [host, port, path] = err_return(connector_type{}());
            auto const connected          = base_type::connect(host, port, path);
            if (spl::failed(connected)) [[unlikely]] {
                auto const* msg = connected.error().message().data();
                return spl::failure("subscription to {} failed: error when connecting ({})", this->id(), msg);
            }
            return spl::success();
        }

        [[nodiscard]] constexpr auto configure(std::chrono::nanoseconds heartbeat,
                                               std::chrono::nanoseconds ping) noexcept -> bool {
            logger::info("{}: Configuring session with heartbeat={} and ping={}", this->id(), heartbeat, ping);
            std::ignore = this->periodic(std::chrono::steady_clock::now(), ping, [this]() -> result<void> {
                if (auto const operation = this->ping(); failed(operation)) [[unlikely]] {
                    return failure("{}", operation.error().message().data());
                }
                return send(spl::protocol::feeder::stream::ping{
                    .timestamp = std::chrono::steady_clock::now().time_since_epoch(),
                });
            });
            std::ignore = this->periodic(std::chrono::steady_clock::now(), heartbeat, [this]() -> result<void> {
                return send(spl::protocol::feeder::stream::heartbeat{
                    .timestamp = std::chrono::steady_clock::now().time_since_epoch(),
                });
            });
            return true;
        }

        template <spl::concepts::object ObjectT>
        [[nodiscard]] constexpr auto send(ObjectT&& object) noexcept -> result<void> {
            logger::debug("{} => {}", this->id(), object);
            clear<spl::components::feeder::direction::outbound>();
            reserve<spl::components::feeder::direction::outbound>(default_capacity);
            return encode(std::forward<ObjectT>(object), buffer<spl::components::feeder::direction ::outbound>());
        }

        template <typename HandlerT>
        [[nodiscard, gnu::hot]] constexpr auto poll(HandlerT&& handler) noexcept -> result<void> {
            auto const intermediary = [&]<typename EventT>(EventT&& event) -> result<void> {
                if constexpr (std::is_same_v<std::decay_t<EventT>, spl::protocol::feeder::stream::ping>) {
                    return this->send(spl::protocol::feeder::stream::pong{
                        .timestamp = std::chrono::steady_clock::now().time_since_epoch(),
                    });
                }
                if constexpr (std::is_same_v<std::decay_t<EventT>, spl::protocol::feeder::stream::heartbeat>) {
                    return this->send(spl::protocol::feeder::stream::heartbeat{
                        .timestamp = std::chrono::steady_clock::now().time_since_epoch(),
                    });
                }
                return handler(std::forward<EventT>(event));
            };

            auto const data = err_return(read());
            if (std::empty(data)) [[unlikely]] {
                return base_type::poll(intermediary);
            }

            logger::debug("{} <= {}", this->id(), std::string_view(std::data(data), std::size(data)));
            err_return(decode(data, handler));
            return base_type::poll(intermediary);
        }

        template <typename InstanceT>
        constexpr auto attach(InstanceT&& instance) noexcept -> void {
            base_type::connector().connection().attach(std::forward<InstanceT>(instance));
        }

    protected:
        template <typename ObjectT, typename BufferT>
        [[nodiscard, gnu::hot]] constexpr auto encode(ObjectT&& object, BufferT& buffer) noexcept -> result<void> {
            err_return(transformer_(object, [&]<typename TransformedT>(TransformedT&& transformed) -> result<void> {
                auto const view  = std::span<char>{std::data(buffer), default_capacity};
                auto const bytes = err_return(encoder_(std::forward<TransformedT>(transformed), view));
                SPL_ASSERT_MSG(bytes <= default_capacity, "Encoder overflow: requires more bytes to encode");
                err_return(write(std::span<char>{std::data(buffer), bytes}));
                logger::debug("{} => {}", this->id(), std::string_view(std::data(buffer), bytes));
                return spl::success();
            }));
            return spl::success();
        }

        template <std::ranges::input_range BufferT, typename HandlerT>
        [[nodiscard, gnu::hot]] constexpr auto decode(BufferT&& buffer, HandlerT&& handler) noexcept -> result<void> {
            auto view = std::span<char const>{std::data(buffer), std::size(buffer)};
            while (not std::empty(view)) {
                auto const transformation = [&]<typename EventT>(EventT&& event) -> result<void> {
                    return transformer_(std::forward<EventT>(event), handler);
                };
                auto const processed = err_return(decoder_(view, transformation));
                view                 = view.subspan(processed);
            }
            return spl::success();
        }

        [[nodiscard, gnu::hot]] constexpr auto read() noexcept -> result<std::span<char const>> {
            auto const available = err_return(base_type::bytes_readable());
            if (available == 0) [[likely]] {
                return std::span<char const>{};
            }
            clear<spl::components::feeder::direction ::inbound>();
            resize<spl::components::feeder::direction ::inbound>(available);
            return read(buffer<spl::components::feeder::direction ::inbound>());
        }

        template <typename BufferT>
        [[nodiscard, gnu::hot]] constexpr auto read(BufferT&& buffered) noexcept -> result<std::span<char const>> {
            auto const bytes = err_return(base_type::read(buffered));
            return span(buffered, bytes);
        }

        template <std::ranges::input_range BufferT>
        [[nodiscard, gnu::hot]] constexpr auto write(BufferT&& buffered) noexcept -> result<std::size_t> {
            if (std::size(buffered) == 0) [[unlikely]] {
                return spl::success();
            }

            auto const write_buffer = boost::asio::buffer(std::data(buffered), std::size(buffered));
            return base_type::write(write_buffer);
        }

        template <spl::components::feeder::direction TypeV>
        [[nodiscard]] constexpr auto buffer() noexcept -> buffer_type<TypeV>& {
            if constexpr (TypeV == spl::components::feeder::direction::inbound) {
                return inbound_buffer_;
            } else {
                return outbound_buffer_;
            }
        }

        template <spl::components::feeder::direction TypeV>
        [[nodiscard]] constexpr auto buffer() const noexcept -> buffer_type<TypeV> const& {
            if constexpr (TypeV == spl::components::feeder::direction::inbound) {
                return inbound_buffer_;
            } else {
                return outbound_buffer_;
            }
        }

        template <typename BufferT>
        [[nodiscard]] constexpr auto view(BufferT&& buffer, std::size_t length) noexcept -> std::string_view {
            if constexpr (requires { buffer.data().data(); }) {
                return {reinterpret_cast<char const*>(buffer.data().data()), length};
            } else {
                return {reinterpret_cast<char const*>(std::data(buffer)), length};
            }
        }

        template <typename BufferT>
        [[nodiscard]] constexpr auto span(BufferT&& buffer, std::size_t length) noexcept -> std::span<char const> {
            if constexpr (requires { buffer.data().data(); }) {
                return {reinterpret_cast<char const*>(buffer.data().data()), length};
            } else {
                return {reinterpret_cast<char const*>(std::data(buffer)), length};
            }
        }

        template <spl::components::feeder::direction TypeV>
        constexpr auto clear() noexcept -> void {
            auto& buffer = this->buffer<TypeV>();
            if constexpr (requires { buffer.clear(); }) {
                buffer.clear();
            }
        }

        template <spl::components::feeder::direction TypeV>
        constexpr auto resize(std::size_t capacity) noexcept -> void {
            auto& buffer = this->buffer<TypeV>();
            if constexpr (requires { buffer.resize(capacity); }) {
                buffer.resize(std::max(capacity, buffer.capacity()));
            }
        }

        template <spl::components::feeder::direction TypeV>
        constexpr auto reserve(std::size_t capacity) noexcept -> void {
            auto& buffer = this->buffer<TypeV>();
            if constexpr (requires { buffer.reserve(capacity); }) {
                buffer.reserve(std::max(capacity, buffer.capacity()));
            }
        }

        constexpr static auto default_capacity = std::size_t{1024};

    private:
        encoder_type encoder_;
        decoder_type decoder_;
        buffer_type<spl::components::feeder::direction::inbound> inbound_buffer_{};
        buffer_type<spl::components::feeder::direction::outbound> outbound_buffer_{};
        transformer_type transformer_{};
    };

} // namespace spl::components::feeder
