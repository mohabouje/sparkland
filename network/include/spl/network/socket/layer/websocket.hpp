#pragma once

#define BOOST_ASIO_SSL_USE_OPENSSL_3

#include "spl/logger/logger.hpp"
#include "spl/network/socket/layer/ssl.hpp"
#include "spl/network/common/error_code.hpp"
#include "spl/network/concepts/socket.hpp"
#include "spl/network/socket/stream.hpp"
#include "spl/result/result.hpp"

#include <boost/beast/websocket.hpp>
#include <boost/asio/ssl/error.hpp>

#include <exception>

namespace spl::network::socket::layer {

    template <concepts::socket SocketT>
    class websocket;

}

namespace boost::beast::websocket {

    template <typename SocketT>
    constexpr auto teardown(role_type role, spl::network::socket::layer::websocket<SocketT>& socket, error_code& error)
        -> void {
        boost::ignore_unused(role, socket, error);
    }

    template <typename SocketT>
    constexpr auto teardown(role_type role, stream<SocketT>& socket, error_code& error) -> void {
        boost::ignore_unused(role, socket, error);
    }

} // namespace boost::beast::websocket

namespace spl::network::socket::layer {

    template <concepts::socket SocketT>
    class websocket {
    public:
        using next_layer_type   = SocketT;
        using protocol_type     = typename next_layer_type::protocol_type;
        using lowest_layer_type = typename next_layer_type::lowest_layer_type;
        using endpoint_type     = typename next_layer_type::endpoint_type;
        using executor_type     = typename next_layer_type::executor_type;

        constexpr explicit websocket(context& context) noexcept;
        constexpr websocket(websocket&& other) noexcept                    = default;
        constexpr auto operator=(websocket&& other) noexcept -> websocket& = default;
        constexpr ~websocket()                                             = default;

        [[nodiscard]] constexpr auto close() noexcept -> result<void>;

        template <typename EndPointIteratorT>
        [[nodiscard]] constexpr auto connect(EndPointIteratorT first, EndPointIteratorT last) noexcept -> result<void>;

        template <typename... IgnoredArgsT>
        [[nodiscard]] constexpr auto configure(std::string const& host, std::string const& port,
                                               std::string const& path, IgnoredArgsT&&... args) noexcept
            -> result<void>;

        [[nodiscard]] constexpr auto get_executor() noexcept -> executor_type {
            return next_layer().lowest_layer().get_executor();
        }

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

        [[nodiscard]] constexpr auto ping(std::string_view raw = "ping") noexcept -> result<void>;

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
        boost::beast::websocket::stream<SocketT> wss_stream_;
    };

    template <concepts::socket SocketT>
    constexpr websocket<SocketT>::websocket(context& context) noexcept : context_(context), wss_stream_(context) {}

    template <concepts::socket SocketT>
    template <typename SettableSocketOption>
    constexpr auto websocket<SocketT>::set_option(SettableSocketOption&& option) noexcept -> result<void> {
        try {
            wss_stream_.set_option(std::forward<SettableSocketOption>(option));
            return success();
        } catch (std::exception& exception) {
            logger::error("Error executing set_option: {}", exception.what());
            return failure(boost::asio::error::invalid_argument);
        }
    }

    template <concepts::socket SocketT>
    template <typename GettableSocketOption>
    constexpr auto websocket<SocketT>::get_option(GettableSocketOption& option) const noexcept -> result<void> {
        try {
            wss_stream_.get_option(std::forward<GettableSocketOption>(option));
            return success();
        } catch (std::exception& exception) {
            logger::error("Error executing get_option: {}", exception.what());
            return failure(boost::asio::error::invalid_argument);
        }
    }

    template <concepts::socket SocketT>
    constexpr auto websocket<SocketT>::local_endpoint() const noexcept -> result<endpoint_type> {
        auto error_code = network::error_code{};
        auto endpoint   = next_layer().local_endpoint(error_code);
        if (error_code) [[unlikely]] {
            return failure(error_code);
        }
        return endpoint;
    }

    template <concepts::socket SocketT>
    constexpr auto websocket<SocketT>::remote_endpoint() const noexcept -> result<endpoint_type> {
        auto error_code = network::error_code{};
        auto endpoint   = next_layer().remote_endpoint(error_code);
        if (error_code) [[unlikely]] {
            return failure(error_code);
        }
        return endpoint;
    }

    template <concepts::socket SocketT>
    constexpr auto websocket<SocketT>::next_layer() const noexcept -> next_layer_type const& {
        return wss_stream_.next_layer();
    }

    template <concepts::socket SocketT>
    constexpr auto websocket<SocketT>::next_layer() noexcept -> next_layer_type& {
        return wss_stream_.next_layer();
    }

    template <concepts::socket SocketT>
    constexpr auto websocket<SocketT>::lowest_layer() const noexcept -> lowest_layer_type const& {
        return wss_stream_.lowest_layer();
    }

    template <concepts::socket SocketT>
    constexpr auto websocket<SocketT>::lowest_layer() noexcept -> lowest_layer_type& {
        return wss_stream_.lowest_layer();
    }

    template <concepts::socket SocketT>
    constexpr auto websocket<SocketT>::available() const noexcept -> result<std::size_t> {
        return next_layer().available();
    }

    template <concepts::socket SocketT>
    constexpr auto websocket<SocketT>::bytes_readable() noexcept -> result<std::size_t> {
        return next_layer().bytes_readable();
    }

    template <concepts::socket SocketT>
    constexpr auto websocket<SocketT>::is_open() const noexcept -> bool {
        return wss_stream_.is_open();
    }

    template <concepts::socket SocketT>
    template <typename ConstBufferSequenceT>
    constexpr auto websocket<SocketT>::write(ConstBufferSequenceT&& buffers, error_code& error_code) noexcept
        -> std::size_t {
        return wss_stream_.write(std::forward<ConstBufferSequenceT>(buffers), error_code);
    }

    template <concepts::socket SocketT>
    template <typename ConstBufferSequenceT>
    constexpr auto websocket<SocketT>::write(ConstBufferSequenceT&& buffers) -> std::size_t {
        return wss_stream_.write(std::forward<ConstBufferSequenceT>(buffers));
    }

    template <concepts::socket SocketT>
    template <typename MutableBufferSequenceT>
    constexpr auto websocket<SocketT>::read(MutableBufferSequenceT&& buffers, error_code& error_code) noexcept
        -> std::size_t {
        return wss_stream_.read(std::forward<MutableBufferSequenceT>(buffers), error_code);
    }

    template <concepts::socket SocketT>
    template <typename MutableBufferSequenceT>
    constexpr auto websocket<SocketT>::read(MutableBufferSequenceT&& buffers) -> std::size_t {
        return wss_stream_.read(std::forward<MutableBufferSequenceT>(buffers));
    }

    template <concepts::socket SocketT>
    template <typename EndPointIteratorT>
    constexpr auto websocket<SocketT>::connect(EndPointIteratorT first, EndPointIteratorT last) noexcept
        -> result<void> {
        return next_layer().connect(first, last);
    }

    template <concepts::socket SocketT>
    template <typename... IgnoredArgsT>
    constexpr auto websocket<SocketT>::configure(std::string const& host, std::string const& port,
                                                 std::string const& path, IgnoredArgsT&&... args) noexcept
        -> result<void> {
        using namespace boost::beast::http;
        using namespace boost::beast::websocket;
        err_return(next_layer().configure(host, port, path, std::forward<IgnoredArgsT>(args)...));

        auto timeout              = stream_base::timeout{};
        timeout.idle_timeout      = std::chrono::seconds(1);
        timeout.handshake_timeout = std::chrono::seconds(5);
        timeout.keep_alive_pings  = true;
        err_return(set_option(timeout));

        try {
            wss_stream_.handshake(std::format("{}:{}", host, port), path);
        } catch (std::exception& exception) {
            logger::error("Error executing handshake: {}", exception.what());
            return failure(boost::asio::ssl::error::unexpected_result);
        }
        return spl::success();
    }

    template <concepts::socket SocketT>
    constexpr auto websocket<SocketT>::close() noexcept -> result<void> {
        auto error_code   = network::error_code{};
        auto const reason = boost::beast::websocket::close_reason(boost::beast::websocket::close_code::normal);
        wss_stream_.close(reason, error_code);
        if (error_code.value() != boost::asio::ssl::error::stream_truncated) [[unlikely]] {
            return failure(error_code);
        }
        return next_layer().close();
    }

    template <concepts::socket SocketT>
    [[nodiscard]] constexpr auto websocket<SocketT>::ping(std::string_view raw) noexcept -> result<void> {
        logger::debug("Sending ping in websocket connection with data=\"{}\"", raw);
        auto error_code = network::error_code{};
        wss_stream_.ping(boost::beast::websocket::ping_data{raw}, error_code);
        if (error_code) [[unlikely]] {
            return failure(error_code);
        }
        return spl::success();
    }

} // namespace spl::network::socket::layer
