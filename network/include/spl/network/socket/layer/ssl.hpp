#pragma once

#include "spl/meta/noncopyable.hpp"
#define BOOST_ASIO_SSL_USE_OPENSSL_3

#include "spl/network/client/tcp.hpp"
#include "spl/network/common/error_code.hpp"
#include "spl/network/concepts/socket.hpp"
#include "spl/result/result.hpp"

#include <boost/beast/core/role.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/host_name_verification.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <string_view>
#include <utility>

namespace spl::network::socket::layer {

    template <concepts::socket SocketT>
    class ssl;
}

namespace boost::beast::websocket {

    inline void teardown(role_type role, spl::network::socket::layer::ssl<spl::network::client::tcp>& socket,
                         error_code& error) {
        ignore_unused(role, socket, error);
    }

} // namespace boost::beast::websocket

namespace spl::network::socket::layer {

    template <concepts::socket SocketT>
    class ssl : public spl::noncopyable {
    public:
        using next_layer_type    = SocketT;
        using protocol_type      = typename next_layer_type::protocol_type;
        using native_handle_type = typename boost::asio::ssl::stream<SocketT>::native_handle_type;
        using lowest_layer_type  = typename next_layer_type::lowest_layer_type;
        using endpoint_type      = typename next_layer_type::endpoint_type;
        using executor_type      = typename next_layer_type::executor_type;
        using ssl_layer_type     = boost::asio::ssl::stream<next_layer_type>;
        using ssl_context_type   = boost::asio::ssl::context;

        constexpr explicit ssl(context& context) noexcept;
        constexpr ssl(ssl&& other) noexcept                    = default;
        constexpr auto operator=(ssl&& other) noexcept -> ssl& = default;
        constexpr ~ssl()                                       = default;

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
        [[nodiscard]] constexpr auto set_option(SettableSocketOption const& option) noexcept -> result<void>;

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

        [[nodiscard]] constexpr auto native_handle() noexcept -> native_handle_type;

        [[nodiscard]] constexpr auto ssl_context() const noexcept -> ssl_context_type const&;

        [[nodiscard]] constexpr auto ssl_context() noexcept -> ssl_context_type&;

        [[nodiscard]] constexpr auto ssl_layer() const noexcept -> ssl_layer_type const&;

        [[nodiscard]] constexpr auto ssl_layer() noexcept -> ssl_layer_type&;

        template <typename ConstBufferSequenceT>
        constexpr auto write_some(ConstBufferSequenceT&& buffers, error_code& error) noexcept -> std::size_t;

        template <typename ConstBufferSequenceT>
        constexpr auto write_some(ConstBufferSequenceT&& buffers) -> std::size_t;

        template <typename MutableBufferSequenceT>
        constexpr auto read_some(MutableBufferSequenceT&& buffers, error_code& error) noexcept -> std::size_t;

        template <typename MutableBufferSequenceT>
        constexpr auto read_some(MutableBufferSequenceT&& buffers) -> std::size_t;

        template <typename ConstBufferSequenceT>
        constexpr auto write(ConstBufferSequenceT&& buffers, error_code& error) noexcept -> std::size_t;

        template <typename ConstBufferSequenceT>
        constexpr auto write(ConstBufferSequenceT&& buffers) -> std::size_t;

        template <typename MutableBufferSequenceT>
        constexpr auto read(MutableBufferSequenceT&& buffers, error_code& error) noexcept -> std::size_t;

        template <typename MutableBufferSequenceT>
        constexpr auto read(MutableBufferSequenceT&& buffers) -> std::size_t;

    private:
        std::reference_wrapper<context> context_;
        boost::asio::ssl::context ssl_context_;
        boost::asio::ssl::stream<next_layer_type> ssl_stream_;
    };

    constexpr auto ssl_debug_callback(const SSL* ssl, int where, int ret) -> void {
        if (ret > 0) [[unlikely]] {
            return;
        }

        auto error_code = SSL_get_error(ssl, ret);
        auto buffer     = std::array<char, 256>{};
        ERR_error_string_n(error_code, std::data(buffer), std::size(buffer));
        logger::trace("SSL Handshake Error (where={}, ret={}): {}", where, ret, std::data(buffer));
    }

    template <concepts::socket SocketT>
    constexpr ssl<SocketT>::ssl(context& context) noexcept :
        context_(context),
        ssl_context_(boost::asio::ssl::context::tlsv12_client),
        ssl_stream_(context_.get(), ssl_context_) {
        ssl_context_.set_default_verify_paths();
        SSL_CTX_set_info_callback(ssl_context_.native_handle(), ssl_debug_callback);
        ERR_print_errors_fp(stderr);
    }

    template <concepts::socket SocketT>
    template <typename SettableSocketOption>
    constexpr auto ssl<SocketT>::set_option(SettableSocketOption const& option) noexcept -> result<void> {
        auto error_code = network::error_code{};
        next_layer().set_option(option, error_code);
        if (error_code) [[unlikely]] {
            return failure(error_code);
        }
        return success();
    }

    template <concepts::socket SocketT>
    template <typename GettableSocketOption>
    constexpr auto ssl<SocketT>::get_option(GettableSocketOption& option) const noexcept -> result<void> {
        auto error_code = network::error_code{};
        next_layer().get_option(option, error_code);
        if (error_code) [[unlikely]] {
            return failure(error_code);
        }
        return success();
    }

    template <concepts::socket SocketT>
    constexpr auto ssl<SocketT>::local_endpoint() const noexcept -> result<endpoint_type> {
        auto error    = error_code{};
        auto endpoint = next_layer().local_endpoint(error);
        if (error) [[unlikely]] {
            return failure(error);
        }
        return endpoint;
    }

    template <concepts::socket SocketT>
    constexpr auto ssl<SocketT>::remote_endpoint() const noexcept -> result<endpoint_type> {
        auto error    = error_code{};
        auto endpoint = next_layer().remote_endpoint(error);
        if (error) [[unlikely]] {
            return failure(error);
        }
        return endpoint;
    }
    template <concepts::socket SocketT>
    constexpr auto ssl<SocketT>::ssl_context() const noexcept -> ssl_context_type const& {
        return ssl_context_;
    }

    template <concepts::socket SocketT>
    constexpr auto ssl<SocketT>::ssl_context() noexcept -> ssl_context_type& {
        return ssl_context_;
    }

    template <concepts::socket SocketT>
    constexpr auto ssl<SocketT>::ssl_layer() const noexcept -> ssl_layer_type const& {
        return ssl_stream_;
    }

    template <concepts::socket SocketT>
    constexpr auto ssl<SocketT>::ssl_layer() noexcept -> ssl_layer_type& {
        return ssl_stream_;
    }

    template <concepts::socket SocketT>
    constexpr auto ssl<SocketT>::next_layer() const noexcept -> next_layer_type const& {
        return ssl_layer().next_layer();
    }

    template <concepts::socket SocketT>
    constexpr auto ssl<SocketT>::next_layer() noexcept -> next_layer_type& {
        return ssl_layer().next_layer();
    }

    template <concepts::socket SocketT>
    constexpr auto ssl<SocketT>::lowest_layer() const noexcept -> lowest_layer_type const& {
        return ssl_layer().lowest_layer();
    }

    template <concepts::socket SocketT>
    constexpr auto ssl<SocketT>::lowest_layer() noexcept -> lowest_layer_type& {
        return ssl_layer().lowest_layer();
    }

    template <concepts::socket SocketT>
    constexpr auto ssl<SocketT>::native_handle() noexcept -> native_handle_type {
        return ssl_layer().native_handle();
    }

    template <concepts::socket SocketT>
    constexpr auto ssl<SocketT>::available() const noexcept -> result<std::size_t> {
        return next_layer().available();
    }

    template <concepts::socket SocketT>
    constexpr auto ssl<SocketT>::bytes_readable() noexcept -> result<std::size_t> {
        return next_layer().bytes_readable();
    }
    template <concepts::socket SocketT>
    constexpr auto ssl<SocketT>::is_open() const noexcept -> bool {
        return next_layer().is_open();
    }

    template <concepts::socket SocketT>
    template <typename ConstBufferSequenceT>
    constexpr auto ssl<SocketT>::write_some(ConstBufferSequenceT&& buffers, error_code& error) noexcept -> std::size_t {
        return ssl_stream_.write_some(buffers, error);
    }

    template <concepts::socket SocketT>
    template <typename ConstBufferSequenceT>
    constexpr auto ssl<SocketT>::write_some(ConstBufferSequenceT&& buffers) -> std::size_t {
        return ssl_stream_.write_some(buffers);
    }

    template <concepts::socket SocketT>
    template <typename MutableBufferSequenceT>
    constexpr auto ssl<SocketT>::read_some(MutableBufferSequenceT&& buffers, error_code& error) noexcept
        -> std::size_t {
        return ssl_stream_.read_some(buffers, error);
    }

    template <concepts::socket SocketT>
    template <typename MutableBufferSequenceT>
    constexpr auto ssl<SocketT>::read_some(MutableBufferSequenceT&& buffers) -> std::size_t {
        return ssl_stream_.read_some(buffers);
    }

    template <concepts::socket SocketT>
    template <typename ConstBufferSequenceT>
    constexpr auto ssl<SocketT>::write(ConstBufferSequenceT&& buffers, error_code& error) noexcept -> std::size_t {
        return ssl_stream_.write_some(std::forward<ConstBufferSequenceT>(buffers), error);
    }

    template <concepts::socket SocketT>
    template <typename ConstBufferSequenceT>
    constexpr auto ssl<SocketT>::write(ConstBufferSequenceT&& buffers) -> std::size_t {
        return ssl_stream_.write_some(std::forward<ConstBufferSequenceT>(buffers));
    }

    template <concepts::socket SocketT>
    template <typename MutableBufferSequenceT>
    constexpr auto ssl<SocketT>::read(MutableBufferSequenceT&& buffers, error_code& error) noexcept -> std::size_t {
        return ssl_stream_.read_some(std::forward<MutableBufferSequenceT>(buffers), error);
    }

    template <concepts::socket SocketT>
    template <typename MutableBufferSequenceT>
    constexpr auto ssl<SocketT>::read(MutableBufferSequenceT&& buffers) -> std::size_t {
        return ssl_stream_.read_some(std::forward<MutableBufferSequenceT>(buffers));
    }

    template <concepts::socket SocketT>
    template <typename EndPointIteratorT>
    [[nodiscard]] constexpr auto ssl<SocketT>::connect(EndPointIteratorT first, EndPointIteratorT last) noexcept
        -> result<void> {
        return next_layer().connect(first, last);
    }

    template <concepts::socket SocketT>
    template <typename... IgnoredArgsT>
    constexpr auto ssl<SocketT>::configure(std::string const& host, std::string const& port, std::string const& path,
                                           IgnoredArgsT&&... args) noexcept -> result<void> {
        err_return(next_layer().configure(host, port, path, std::forward<IgnoredArgsT>(args)...));

        if (!SSL_set_tlsext_host_name(ssl_layer().native_handle(), host.c_str())) {
            return failure(boost::asio::error::invalid_argument);
        }

        auto error = error_code{};
        if (ssl_layer().handshake(boost::asio::ssl::stream_base::client, error); error.failed()) [[unlikely]] {
            return failure(error);
        }

        return success();
    }

    template <concepts::socket SocketT>
    constexpr auto ssl<SocketT>::close() noexcept -> result<void> {
        auto error_code = network::error_code{};
        ssl_layer().shutdown(error_code);
        if (error_code.value() != boost::asio::ssl::error::stream_truncated) [[unlikely]] {
            return failure(error_code);
        }

        return next_layer().close();
    }

} // namespace spl::network::socket::layer
