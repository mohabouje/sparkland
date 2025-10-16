#pragma once

#include "spl/logger/logger.hpp"
#include "spl/network/common/error_code.hpp"
#include "spl/network/common/context.hpp"
#include "spl/network/concepts/socket.hpp"
#include "spl/meta/noncopyable.hpp"
#include "spl/result/result.hpp"

#include <boost/asio/ip/basic_resolver.hpp>

namespace spl::network::connector {

    template <concepts::socket SocketT>
    class router {
    public:
        using stream_type        = typename std::remove_reference<SocketT>::type;
        using protocol_type      = typename stream_type::protocol_type;
        using endpoint_type      = typename stream_type::endpoint_type;
        using resolver_type      = boost::asio::ip::basic_resolver<protocol_type>;
        using resolve_flags_type = boost::asio::ip::resolver_base::flags;
        using query_type         = std::tuple<protocol_type, std::string, std::string, resolve_flags_type>;
        using results_type       = typename resolver_type::results_type;

        explicit constexpr router(context& context) noexcept;
        constexpr router(router&&) noexcept                         = default;
        constexpr auto operator=(router&&) noexcept -> router&      = default;
        constexpr router(router const&) noexcept                    = default;
        constexpr auto operator=(router const&) noexcept -> router& = default;
        constexpr ~router() noexcept                                = default;

        constexpr auto make_connection(std::string const& host, std::string const& port,
                                       std::string const& path) noexcept -> result<SocketT>;

        constexpr auto make_connection(std::optional<SocketT>& client, std::string const& host, std::string const& port,
                                       std::string const& path) noexcept -> result<void>;

        constexpr auto resolve(std::string_view host, std::string_view port) noexcept -> result<results_type>;

        constexpr auto connect(SocketT& client, std::string const& host, std::string const& port,
                               std::string const& path) noexcept -> result<void>;

    private:
        std::reference_wrapper<context> context_;
        resolver_type resolver_;
    };

    template <concepts::socket SocketT>
    constexpr router<SocketT>::router(context& context) noexcept : context_(context), resolver_(context_.get()) {}

    template <concepts::socket SocketT>
    constexpr auto router<SocketT>::resolve(std::string_view host, std::string_view port) noexcept
        -> result<typename router<SocketT>::results_type> {
        logger::info("Trying to resolve endpoints for {}:{}", host, port);
        auto error_code = network::error_code{};
        auto endpoints  = resolver_.resolve(host, port, resolve_flags_type::address_configured, error_code);
        return endpoints;
    }

    template <concepts::socket SocketT>
    constexpr auto router<SocketT>::connect(SocketT& client, std::string const& host, std::string const& port,
                                            std::string const& path) noexcept -> result<void> {
        auto const endpoints = err_return(this->resolve(host, port));
        logger::info("Trying to connect to {}:{} path={} to one of the resolved {} endpoints", host, port, path,
                     std::size(endpoints));
        err_return(client.connect(std::begin(endpoints), std::end(endpoints)));
        logger::info("Connection to server initialized, redirecting to path={}", path);
        err_return(client.configure(host, port, path));
        logger::info("Connection to {}:{} established successfully", host, port);
        return success();
    }

    template <concepts::socket SocketT>
    constexpr auto router<SocketT>::make_connection(std::string const& host, std::string const& port,
                                                    std::string const& path) noexcept -> result<SocketT> {
        auto client = SocketT(context_.get());
        err_return(connect(client, host, port, path));
        return client;
    }

    template <concepts::socket SocketT>
    constexpr auto router<SocketT>::make_connection(std::optional<SocketT>& client, std::string const& host,
                                                    std::string const& port, std::string const& path) noexcept
        -> result<void> {
        client.emplace(context_.get());
        return connect(*client, host, port, path);
    }

} // namespace spl::network::connector