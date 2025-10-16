#pragma once

#include "boost/asio/ip/tcp.hpp"
#include "spl/network/common/context.hpp"
#include "spl/logger/logger.hpp"
#include "spl/network/common/error_code.hpp"
#include "spl/network/common/result.hpp"

#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/role.hpp>

#include <ranges>

namespace spl::network::socket {

    template <typename SocketT>
    class stream;
}

namespace boost::beast::websocket {

    inline void teardown(role_type role, spl::network::socket::stream<asio::ip::tcp>& socket,
                         system::error_code& error) {
        ignore_unused(role, socket, error);
    }

} // namespace boost::beast::websocket

namespace spl::network::socket {

    template <typename ProtocolT>
    class stream {
    public:
        using next_layer_type   = boost::asio::basic_stream_socket<ProtocolT>;
        using protocol_type     = typename next_layer_type::protocol_type;
        using lowest_layer_type = typename next_layer_type::lowest_layer_type;
        using endpoint_type     = typename next_layer_type::endpoint_type;
        using executor_type     = typename next_layer_type::executor_type;

        constexpr static auto receive_buffer_size = 2048;
        constexpr static auto send_buffer_size    = 2048;

        constexpr explicit stream(context& context) noexcept;
        constexpr stream(stream&& other) noexcept                    = default;
        constexpr auto operator=(stream&& other) noexcept -> stream& = default;
        constexpr ~stream()                                          = default;

        [[nodiscard]] constexpr auto close() noexcept -> result<void>;

        template <typename EndPointIteratorT>
        [[nodiscard]] constexpr auto connect(EndPointIteratorT first, EndPointIteratorT last) noexcept -> result<void>;

        template <typename... IgnoredArgsT>
        [[nodiscard]] constexpr auto configure(IgnoredArgsT&&... args) noexcept -> result<void>;

        template <typename SettableSocketOption>
        [[nodiscard]] constexpr auto set_option(SettableSocketOption&& option) noexcept -> result<void>;

        template <typename GettableSocketOption>
        [[nodiscard]] constexpr auto get_option(GettableSocketOption& option) const noexcept -> result<void>;

        [[nodiscard]] constexpr auto local_endpoint() const noexcept -> result<endpoint_type>;

        [[nodiscard]] constexpr auto remote_endpoint() const noexcept -> result<endpoint_type>;

        [[nodiscard]] constexpr auto available() const noexcept -> result<std::size_t>;

        [[nodiscard]] constexpr auto bytes_readable() noexcept -> result<std::size_t>;

        [[nodiscard]] constexpr auto next_layer() const noexcept -> next_layer_type const&;

        [[nodiscard]] constexpr auto next_layer() noexcept -> next_layer_type&;

        [[nodiscard]] constexpr auto lowest_layer() const noexcept -> lowest_layer_type const&;

        [[nodiscard]] constexpr auto lowest_layer() noexcept -> lowest_layer_type&;

        [[nodiscard]] constexpr auto is_open() const noexcept -> bool;

        [[nodiscard]] constexpr auto get_executor() noexcept -> executor_type {
            return next_layer().lowest_layer().get_executor();
        }

        template <typename ConstBufferSequenceT>
        constexpr auto write_some(ConstBufferSequenceT&& buffers, error_code& error_code) noexcept -> std::size_t;

        template <typename ConstBufferSequenceT>
        constexpr auto write_some(ConstBufferSequenceT&& buffers) -> std::size_t;

        template <typename MutableBufferSequenceT>
        constexpr auto read_some(MutableBufferSequenceT&& buffers, error_code& error_code) noexcept -> std::size_t;

        template <typename MutableBufferSequenceT>
        constexpr auto read_some(MutableBufferSequenceT&& buffers) -> std::size_t;

        template <typename ConstBufferSequenceT>
        constexpr auto write(ConstBufferSequenceT&& buffers, error_code& error_code) noexcept -> std::size_t;

        template <typename ConstBufferSequenceT>
        constexpr auto write(ConstBufferSequenceT&& buffers) -> std::size_t;

        template <typename MutableBufferSequenceT>
        constexpr auto read(MutableBufferSequenceT&& buffers, error_code& error_code) noexcept -> std::size_t;

        template <typename MutableBufferSequenceT>
        constexpr auto read(MutableBufferSequenceT&& buffers) -> std::size_t;

    private:
        std::reference_wrapper<context> context_;
        boost::asio::basic_stream_socket<ProtocolT> stream_socket_;
        boost::asio::socket_base::bytes_readable command_{0};
    };

    template <typename ProtocolT>
    constexpr stream<ProtocolT>::stream(context& context) noexcept : context_(context), stream_socket_(context) {
        if (auto operation = set_option(boost::asio::socket_base::keep_alive(true))) {
            logger::error("Error while setting keep alive option for stream: {}", operation.error().message().data());
        }
    }

    template <typename ProtocolT>
    template <typename EndPointIteratorT>
    [[nodiscard]] constexpr auto stream<ProtocolT>::connect(EndPointIteratorT first, EndPointIteratorT last) noexcept
        -> result<void> {
        auto error_code = network::error_code{};
        boost::asio::connect(this->stream_socket_, first, last, error_code);
        if (error_code.failed()) [[unlikely]] {
            return failure(error_code);
        }
        return success();
    }

    template <typename ProtocolT>
    template <typename... IgnoredArgsT>
    constexpr auto stream<ProtocolT>::configure([[maybe_unused]] IgnoredArgsT&&... args) noexcept -> result<void> {
        // // @note https://stackoverflow.com/a/35434596
        // err_return(set_option(boost::asio::socket_base::receive_buffer_size(receive_buffer_size)));
        // err_return(set_option(boost::asio::socket_base::send_buffer_size(send_buffer_size)));
        return success();
    }

    template <typename ProtocolT>
    constexpr auto stream<ProtocolT>::close() noexcept -> result<void> {
        auto error_code = network::error_code{};
        stream_socket_.close(error_code);
        if (error_code) [[unlikely]] {
            return failure(error_code);
        }
        return success();
    }

    template <typename ProtocolT>
    template <typename SettableSocketOption>
    constexpr auto stream<ProtocolT>::set_option(SettableSocketOption&& option) noexcept -> result<void> {
        auto error_code = network::error_code{};
        stream_socket_.set_option(std::forward<SettableSocketOption>(option), error_code);
        if (error_code.failed()) [[unlikely]] {
            return failure(error_code);
        }
        return success();
    }

    template <typename ProtocolT>
    template <typename GettableSocketOption>
    constexpr auto stream<ProtocolT>::get_option(GettableSocketOption& option) const noexcept -> result<void> {
        auto error_code = network::error_code{};
        stream_socket_.get_option(option, error_code);
        if (error_code.failed()) [[unlikely]] {
            return failure(error_code);
        }
        return success();
    }

    template <typename ProtocolT>
    constexpr auto stream<ProtocolT>::local_endpoint() const noexcept -> result<endpoint_type> {
        auto error_code = network::error_code{};
        auto endpoint   = stream_socket_.local_endpoint(error_code);
        if (error_code.failed()) [[unlikely]] {
            return failure(error_code);
        }
        return endpoint;
    }

    template <typename ProtocolT>
    constexpr auto stream<ProtocolT>::remote_endpoint() const noexcept -> result<endpoint_type> {
        auto error_code = network::error_code{};
        auto endpoint   = stream_socket_.remote_endpoint(error_code);
        if (error_code.failed()) [[unlikely]] {
            return failure(error_code);
        }
        return endpoint;
    }

    template <typename ProtocolT>
    constexpr auto stream<ProtocolT>::next_layer() const noexcept -> next_layer_type const& {
        return stream_socket_;
    }

    template <typename ProtocolT>
    constexpr auto stream<ProtocolT>::next_layer() noexcept -> next_layer_type& {
        return stream_socket_;
    }

    template <typename ProtocolT>
    constexpr auto stream<ProtocolT>::lowest_layer() const noexcept -> lowest_layer_type const& {
        return stream_socket_.lowest_layer();
    }

    template <typename ProtocolT>
    constexpr auto stream<ProtocolT>::lowest_layer() noexcept -> lowest_layer_type& {
        return stream_socket_.lowest_layer();
    }

    template <typename ProtocolT>
    constexpr auto stream<ProtocolT>::available() const noexcept -> result<std::size_t> {
        error_code error_code{};
        auto const result = stream_socket_.available(error_code);
        if (error_code) [[unlikely]] {
            return failure(error_code);
        }
        return result;
    }

    template <typename ProtocolT>
    constexpr auto stream<ProtocolT>::bytes_readable() noexcept -> result<std::size_t> {
        error_code error_code{};
        stream_socket_.io_control(command_, error_code);
        if (error_code) [[unlikely]] {
            return failure(error_code);
        }
        return command_.get();
    }

    template <typename ProtocolT>
    constexpr auto stream<ProtocolT>::is_open() const noexcept -> bool {
        return stream_socket_.is_open();
    }

    template <typename ProtocolT>
    template <typename ConstBufferSequenceT>
    constexpr auto stream<ProtocolT>::write_some(ConstBufferSequenceT&& buffers, error_code& error_code) noexcept
        -> std::size_t {
        return stream_socket_.write_some(buffers, error_code);
    }

    template <typename ProtocolT>
    template <typename ConstBufferSequenceT>
    constexpr auto stream<ProtocolT>::write_some(ConstBufferSequenceT&& buffers) -> std::size_t {
        return stream_socket_.write_some(buffers);
    }

    template <typename ProtocolT>
    template <typename MutableBufferSequenceT>
    constexpr auto stream<ProtocolT>::read_some(MutableBufferSequenceT&& buffers, error_code& error_code) noexcept
        -> std::size_t {
        return stream_socket_.read_some(buffers, error_code);
    }

    template <typename ProtocolT>
    template <typename MutableBufferSequenceT>
    constexpr auto stream<ProtocolT>::read_some(MutableBufferSequenceT&& buffers) -> std::size_t {
        return stream_socket_.read_some(buffers);
    }

    template <typename ProtocolT>
    template <typename ConstBufferSequenceT>
    constexpr auto stream<ProtocolT>::write(ConstBufferSequenceT&& buffers, error_code& error_code) noexcept
        -> std::size_t {
        return stream_socket_.write(std::forward<ConstBufferSequenceT>(buffers), error_code);
    }

    template <typename ProtocolT>
    template <typename ConstBufferSequenceT>
    constexpr auto stream<ProtocolT>::write(ConstBufferSequenceT&& buffers) -> std::size_t {
        return stream_socket_.write(std::forward<ConstBufferSequenceT>(buffers));
    }

    template <typename ProtocolT>
    template <typename MutableBufferSequenceT>
    constexpr auto stream<ProtocolT>::read(MutableBufferSequenceT&& buffers, error_code& error_code) noexcept
        -> std::size_t {
        return stream_socket_.read(std::forward<MutableBufferSequenceT>(buffers), error_code);
    }

    template <typename ProtocolT>
    template <typename MutableBufferSequenceT>
    constexpr auto stream<ProtocolT>::read(MutableBufferSequenceT&& buffers) -> std::size_t {
        return stream_socket_.read(std::forward<MutableBufferSequenceT>(buffers));
    }

} // namespace spl::network::socket
