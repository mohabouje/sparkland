#pragma once

#include "spl/network/common/result.hpp"

#include <utility>
#include <optional>

namespace spl::network::concepts {

    template <typename ImplementationT>
    concept connectable = requires(ImplementationT socket, typename ImplementationT::endpoint_type const& endpoint,
                                   std::optional<std::chrono::nanoseconds> duration) {
        // { socket.connect(endpoint) } noexcept -> std::same_as<result<void>>;
        { socket.close() } noexcept -> std::same_as<result<void>>;
        { socket.is_open() } noexcept -> std::same_as<bool>;
    };

    template <typename ImplementationT>
    concept layerable = requires(ImplementationT socket) {
        typename ImplementationT::protocol_type;
        typename ImplementationT::endpoint_type;
        typename ImplementationT::lowest_layer_type;
        typename ImplementationT::lowest_layer_type;
        typename ImplementationT::next_layer_type;
        { socket.local_endpoint() } noexcept -> std::same_as<result<typename ImplementationT::endpoint_type>>;
        { socket.remote_endpoint() } noexcept -> std::same_as<result<typename ImplementationT::endpoint_type>>;
        { socket.next_layer() } noexcept -> std::same_as<typename ImplementationT::next_layer_type&>;
        {
            std::as_const(socket).next_layer()
        } noexcept -> std::same_as<typename ImplementationT::next_layer_type const&>;
        { socket.next_layer() } noexcept -> std::same_as<typename ImplementationT::next_layer_type&>;
        {
            std::as_const(socket).lowest_layer()
        } noexcept -> std::same_as<typename ImplementationT::lowest_layer_type const&>;
        { socket.lowest_layer() } noexcept -> std::same_as<typename ImplementationT::lowest_layer_type&>;
    };

    template <typename ImplementationT>
    concept socket = connectable<ImplementationT> /* && layerable<ImplementationT> */;

    namespace internal {
        template <typename...>
        struct is_socket_implementation : public std::false_type {};

        template <socket ImplementationT>
        struct is_socket_implementation<ImplementationT> : public std::false_type {};
    } // namespace internal

    template <typename ImplementationT>
    struct is_socket : public internal::is_socket_implementation<ImplementationT> {};

    template <typename ImplementationT>
    constexpr auto is_socket_v = is_socket<ImplementationT>::value;

} // namespace spl::network::concepts
