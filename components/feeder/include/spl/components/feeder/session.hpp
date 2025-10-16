#pragma once

#include "spl/components/feeder/session_id.hpp"
#include "spl/components/scheduler/scheduler.hpp"
#include "spl/network/connector/connector.hpp"
#include "spl/network/common/error_code.hpp"
#include "spl/network/common/result.hpp"
#include "spl/network/concepts/socket.hpp"
#include "spl/network/connector/router.hpp"
#include "spl/network/connector/connector.hpp"
#include "spl/logger/logger.hpp"
#include "spl/result/result.hpp"
#include "spl/meta/typeinfo.hpp"

#include <boost/beast/core/flat_buffer.hpp>
#include <boost/asio/streambuf.hpp>

#include <type_traits>
#include <utility>
#include <functional>

namespace spl::components::feeder {

    template <typename ContractT>
    struct session : public components::scheduler::scheduler {
        using contract_type   = std::decay_t<ContractT>;
        using connection_type = typename contract_type::connection_type;
        using connector_type  = spl::network::connector::connector<connection_type>;

        constexpr session(spl::network::context& context, spl::components::feeder::session_id const& sid) noexcept :
            context_(context), session_id_(sid), connector_(context, session_id_.connection_id()) {}

        constexpr session(session const& other) noexcept            = default;
        constexpr session& operator=(session const& other) noexcept = default;
        constexpr session(session&& other) noexcept                 = default;
        constexpr session& operator=(session&& other) noexcept      = default;
        constexpr ~session() noexcept {
            std::ignore = this->disconnect();
        }

        template <typename... ArgsT>
        [[nodiscard]] constexpr auto connect(ArgsT&&... args) noexcept -> result<void> {
            if (this->ready()) [[unlikely]] {
                logger::debug("Session {}: ignoring connect request, already connected", id());
                return success();
            }

            logger::info("Session {}: trying to connect the session", id());
            std::ignore = connector_.set_endpoint(std::forward<ArgsT>(args)...);
            if (auto const operation = connector_.connect(); spl::failed(operation)) [[unlikely]] {
                return spl::failure("Session {} [connect]: \"{}\"", id(), operation.error().message().data());
            }
            return spl::success();
        }

        [[nodiscard]] constexpr auto disconnect() noexcept -> result<void> {
            if (not this->ready()) [[unlikely]] {
                logger::debug("Session {}: ignoring disconnect request, already disconnected", id());
                return success();
            }

            logger::info("Session {}: trying to disconnect the session", id());
            if (auto const operation = connector_.disconnect(); spl::failed(operation)) [[unlikely]] {
                return spl::failure("Session {} [disconnect]: \"{}\"", id(), operation.error().message().data());
            }
            return spl::success();
        }

        template <typename HandlerT>
        [[nodiscard, gnu::hot]] constexpr auto poll(HandlerT&& handler) noexcept -> result<void> {
            if (auto const operation = connector_.poll(handler); spl::failed(operation)) [[unlikely]] {
                return spl::failure("Session {} [poll]: \"{}\"", id(), operation.error().message().data());
            }
            return components::scheduler::scheduler::poll(std::forward<HandlerT>(handler));
        }

        [[nodiscard]] constexpr auto id() const noexcept -> std::size_t {
            return session_id_.token_id();
        }

        [[nodiscard]] constexpr auto status() const noexcept -> spl::network::status {
            return connector_.status();
        }

        [[nodiscard]] constexpr auto ready() const noexcept -> bool {
            return connector_.ready();
        }

        [[nodiscard]] explicit constexpr operator bool() const noexcept {
            return static_cast<bool>(connector_);
        }

        [[nodiscard]] constexpr auto cache() noexcept -> boost::asio::streambuf& {
            return connector_.connection().cache();
        }

        [[nodiscard]] constexpr auto ping() noexcept -> spl::network::result<void> {
            return connector_.ping();
        }

        [[nodiscard]] constexpr auto connection() noexcept -> connection_type& {
            return connector_.connection();
        }

        [[nodiscard]] constexpr auto connection() const noexcept -> connection_type const& {
            return connector_.connection();
        }

        [[nodiscard]] constexpr auto connector() noexcept -> connector_type& {
            return connector_;
        }

        [[nodiscard]] constexpr auto connector() const noexcept -> connector_type const& {
            return connector_;
        }

    protected:
        template <typename MutableBufferSequenceT>
        [[nodiscard, gnu::hot]] constexpr auto read(MutableBufferSequenceT&& buffer) -> result<std::size_t> {
            if (not ready()) [[unlikely]] {
                return 0;
            }

            auto const operation = connector_.read(std::forward<MutableBufferSequenceT>(buffer));
            if (spl::failed(operation)) [[unlikely]] {
                return spl::failure("Session {} [read]: \"{}\"", id(), operation.error().message().data());
            }
            return operation.value();
        }

        template <typename ConstBufferSequenceT>
        [[nodiscard, gnu::hot]] constexpr auto write(ConstBufferSequenceT&& object) -> result<std::size_t> {
            if (not ready()) [[unlikely]] {
                return 0;
            }

            auto const operation = connector_.write(std::forward<ConstBufferSequenceT>(object));
            if (spl::failed(operation)) [[unlikely]] {
                return spl::failure("Session {} [write]: \"{}\"", id(), operation.error().message().data());
            }
            return operation.value();
        }

        [[nodiscard, gnu::hot]] constexpr auto bytes_readable() noexcept -> result<std::size_t> {
            if (not ready()) [[unlikely]] {
                return 0;
            }

            auto const operation = connector_.bytes_readable();
            if (spl::failed(operation)) [[unlikely]] {
                return spl::failure("Session {} [bytes_readable]: \"{}\"", id(), operation.error().message().data());
            }
            return operation.value();
        }

    private:
        std::reference_wrapper<spl::network::context> context_;
        spl::components::feeder::session_id session_id_;
        spl::network::connector::connector<connection_type> connector_;
    };

} // namespace spl::components::feeder