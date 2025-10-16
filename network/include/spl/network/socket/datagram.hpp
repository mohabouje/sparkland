#pragma once

#include "spl/network/common/error_code.hpp"
#include "spl/network/common/context.hpp"
#include "spl/network/common/result.hpp"

#include <boost/asio/basic_datagram_socket.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/connect.hpp>

namespace spl::network::socket {

    template <typename ProtocolT>
    class datagram {
    public:
        using next_layer_type   = boost::asio::basic_datagram_socket<ProtocolT>;
        using protocol_type     = typename next_layer_type::protocol_type;
        using lowest_layer_type = typename next_layer_type::lowest_layer_type;
        using endpoint_type     = typename next_layer_type::endpoint_type;
        using executor_type     = typename next_layer_type::executor_type;

        constexpr explicit datagram(context& context) noexcept;
        constexpr datagram(datagram&& other) noexcept                    = default;
        constexpr auto operator=(datagram&& other) noexcept -> datagram& = default;
        constexpr ~datagram()                                            = default;

        [[nodiscard]] constexpr auto close() noexcept -> result<void>;

        template <typename EndPointIteratorT>
        [[nodiscard]] constexpr auto connect(EndPointIteratorT first, EndPointIteratorT last) noexcept -> result<void>;

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

        template <typename MutableBufferSequenceT>
        constexpr auto read_some(MutableBufferSequenceT&& buffers, error_code& error_code) noexcept -> std::size_t;

    private:
        std::reference_wrapper<context> context_;
        boost::asio::basic_datagram_socket<ProtocolT> datagram_socket_;
        boost::asio::socket_base::bytes_readable command_{true};
    };

    template <typename ProtocolT>
    constexpr datagram<ProtocolT>::datagram(context& context) noexcept : context_(context), datagram_socket_(context) {
        std::ignore = set_option(boost::asio::socket_base::keep_alive(true));
        std::ignore = set_option(boost::asio::socket_base::broadcast(true));
    }

    template <typename ProtocolT>
    constexpr auto datagram<ProtocolT>::close() noexcept -> result<void> {
        auto error_code = network::error_code{};
        datagram_socket_.close(error_code);
        if (error_code) [[unlikely]] {
            return failure(error_code);
        }
        return success();
    }

    template <typename ProtocolT>
    template <typename SettableSocketOption>
    constexpr auto datagram<ProtocolT>::set_option(SettableSocketOption&& option) noexcept -> result<void> {
        auto error_code = network::error_code{};
        datagram_socket_.set_option(std::forward<SettableSocketOption>(option), error_code);
        if (error_code) [[unlikely]] {
            return failure(error_code);
        }
        return success();
    }

    template <typename ProtocolT>
    template <typename GettableSocketOption>
    constexpr auto datagram<ProtocolT>::get_option(GettableSocketOption& option) const noexcept -> result<void> {
        auto error_code = network::error_code{};
        datagram_socket_.get_option(option, error_code);
        if (error_code) [[unlikely]] {
            return failure(error_code);
        }
        return success();
    }

    template <typename ProtocolT>
    constexpr auto datagram<ProtocolT>::local_endpoint() const noexcept -> result<endpoint_type> {
        auto error_code = network::error_code{};
        auto endpoint   = datagram_socket_.local_endpoint(error_code);
        if (error_code) [[unlikely]] {
            return failure(error_code);
        }
        return endpoint;
    }

    template <typename ProtocolT>
    constexpr auto datagram<ProtocolT>::remote_endpoint() const noexcept -> result<endpoint_type> {
        auto error_code = network::error_code{};
        auto endpoint   = datagram_socket_.remote_endpoint(error_code);
        if (error_code) [[unlikely]] {
            return failure(error_code);
        }
        return endpoint;
    }

    template <typename ProtocolT>
    constexpr auto datagram<ProtocolT>::next_layer() const noexcept -> next_layer_type const& {
        return datagram_socket_;
    }

    template <typename ProtocolT>
    constexpr auto datagram<ProtocolT>::next_layer() noexcept -> next_layer_type& {
        return datagram_socket_;
    }

    template <typename ProtocolT>
    constexpr auto datagram<ProtocolT>::lowest_layer() const noexcept -> lowest_layer_type const& {
        return datagram_socket_.lowest_layer();
    }

    template <typename ProtocolT>
    constexpr auto datagram<ProtocolT>::lowest_layer() noexcept -> lowest_layer_type& {
        return datagram_socket_.lowest_layer();
    }

    template <typename ProtocolT>
    constexpr auto datagram<ProtocolT>::available() const noexcept -> result<std::size_t> {
        error_code error_code{};
        auto const result = datagram_socket_.available(error_code);
        if (error_code) [[unlikely]] {
            return failure(error_code);
        }
        return result;
    }

    template <typename ProtocolT>
    constexpr auto datagram<ProtocolT>::bytes_readable() noexcept -> result<std::size_t> {
        error_code error_code{};
        datagram_socket_.io_control(command_, error_code);
        if (error_code) [[unlikely]] {
            return failure(error_code);
        }
        return command_.get();
    }

    template <typename ProtocolT>
    constexpr auto datagram<ProtocolT>::is_open() const noexcept -> bool {
        return datagram_socket_.is_open();
    }

    template <typename ProtocolT>
    template <typename ConstBufferSequenceT>
    constexpr auto datagram<ProtocolT>::write_some(ConstBufferSequenceT&& buffers, error_code& error_code) noexcept
        -> std::size_t {
        return datagram_socket_.write_some(buffers, error_code);
    }

    template <typename ProtocolT>
    template <typename MutableBufferSequenceT>
    constexpr auto datagram<ProtocolT>::read_some(MutableBufferSequenceT&& buffers, error_code& error_code) noexcept
        -> std::size_t {
        return datagram_socket_.read_some(buffers, error_code);
    }

    template <typename ProtocolT>
    template <typename EndPointIteratorT>
    constexpr auto datagram<ProtocolT>::connect(EndPointIteratorT first, EndPointIteratorT last) noexcept
        -> result<void> {
        auto error_code = network::error_code{};
        boost::asio::connect(this->datagram_socket_, first, last, error_code);
        if (error_code.failed()) [[unlikely]] {
            return failure(error_code);
        }
        return success();
    }

} // namespace spl::network::socket
