#pragma once

#include "spl/core/assert.hpp"
#include "spl/network/common/context.hpp"
#include "spl/network/common/error_code.hpp"
#include "spl/network/common/result.hpp"
#include "spl/network/connector/router.hpp"
#include "spl/network/status.hpp"
#include "spl/network/connection_id.hpp"
#include "spl/result/result.hpp"

#include <chrono>
#include <ratio>
#include <system_error>

namespace spl::network::connector {

    struct scheduler {
        struct configuration {
            std::size_t maximum_attempts{10};
            std::chrono::milliseconds minimum_backoff{500};
            std::chrono::milliseconds maximum_backoff{300'000};
        };

        constexpr scheduler(spl::network::connection_id const& id, configuration const& config) noexcept :
            id_(id), configuration_(config) {
            logger::info("Scheduler[{}] Configured with {}", id, configuration_);
        }

        [[nodiscard]] constexpr auto schedule(std::chrono::steady_clock::time_point const& now) noexcept
            -> result<void> {
            if (current_attempt_ >= configuration_.maximum_attempts) {
                return spl::failure(boost::asio::error::basic_errors::operation_aborted);
            }

            auto const max      = configuration_.maximum_attempts;
            auto const next     = configuration_.minimum_backoff.count() * std::pow(2, current_attempt_++);
            auto const current  = std::chrono::milliseconds(static_cast<std::size_t>(next));
            auto const delay    = std::min(current, configuration_.maximum_backoff);
            current_expiration_ = now + delay;

            auto const elapsed = current_expiration_->time_since_epoch();
            auto const counter = current_attempt_;
            logger::info("Scheduler[{}] Attempt {}/{} to reconnect scheduled in {}", id(), counter, max, elapsed);
            return spl::success();
        }

        [[nodiscard]] constexpr auto scheduled() const noexcept -> bool {
            return current_expiration_.has_value();
        }

        [[nodiscard]] constexpr auto expired(std::chrono::steady_clock::time_point const& now) const noexcept -> bool {
            return current_expiration_ and now >= current_expiration_.value();
        }

        [[nodiscard]] constexpr explicit operator bool() const noexcept {
            return current_expiration_.has_value();
        }

        [[nodiscard]] constexpr auto poll(std::chrono::steady_clock::time_point const& now) noexcept -> bool {
            if (not expired(now)) [[unlikely]] {
                return false;
            }

            current_expiration_.reset();
            return true;
        }

        [[nodiscard]] constexpr auto reset() noexcept {
            auto const max = configuration_.maximum_attempts;
            logger::info("Scheduler[{}] Reset the reconnection scheduler: {}/{}", id(), current_attempt_, max);
            current_attempt_    = 0;
            current_expiration_ = std::nullopt;
            return spl::success();
        }

        [[nodiscard]] constexpr auto id() const noexcept -> spl::network::connection_id const& {
            return id_;
        }

    private:
        spl::network::connection_id id_;
        configuration configuration_;
        std::size_t current_attempt_{0};
        std::optional<std::chrono::steady_clock::time_point> current_expiration_{};
    };

    struct endpoint {
        std::string host;
        std::string port;
        std::string path;
    };

    template <concepts::socket ConnectionT>
    struct connector {
        using connection_type = std::decay_t<ConnectionT>;
        using router_type     = spl::network::connector::router<connection_type>;

        constexpr connector(spl::network::context& context, spl::network::connection_id const& id,
                            scheduler::configuration const& configuration = {}) :
            router_(context), id_(id), scheduler_(id, configuration) {}

        constexpr connector(connector const& other) noexcept            = default;
        constexpr connector(connector&& other) noexcept                 = default;
        constexpr connector& operator=(connector&& other) noexcept      = default;
        constexpr connector& operator=(connector const& other) noexcept = default;

        [[nodiscard]] constexpr auto connection() const noexcept -> connection_type const& {
            return *connection_;
        }

        [[nodiscard]] constexpr auto connection() noexcept -> connection_type& {
            return *connection_;
        }

        [[nodiscard]] constexpr auto router() const noexcept -> router_type const& {
            return router_;
        }

        [[nodiscard]] constexpr auto router() noexcept -> router_type& {
            return router_;
        }

        template <typename... ArgsT>
        [[nodiscard]] constexpr auto set_endpoint(ArgsT&&... args) noexcept -> result<void> {
            endpoint_.emplace(endpoint{std::forward<ArgsT>(args)...});
            logger::info("Connection [{}] configured for endpoint={}", id(), *endpoint_);
            return spl::success();
        }

        [[nodiscard]] constexpr auto connect() noexcept -> result<void> {
            SPL_ASSERT_MSG(endpoint_, "trying to connect without an endpoint");
            logger::info("Connection [{}] Trying to connect...", id());
            set_status(spl::network::status::connecting);
            auto operation = router_.make_connection(connection_, endpoint_->host, endpoint_->port, endpoint_->path);
            if (spl::failed(operation)) [[unlikely]] {
                err_return(intercept(operation, "connect"));
                return spl::success();
            }
            set_status(spl::network::status::connected);
            logger::info("Connection [{}] Connection established", id());
            return spl::success();
        }

        [[nodiscard]] constexpr auto reconnect() noexcept -> result<void> {
            SPL_ASSERT_MSG(endpoint_, "trying to connect without an endpoint");
            logger::info("Connection [{}] Trying to reconnect...", id());
            set_status(spl::network::status::reconnecting);
            auto operation = router_.make_connection(connection_, endpoint_->host, endpoint_->port, endpoint_->path);
            if (spl::failed(operation)) [[unlikely]] {
                err_return(intercept(operation, "reconnect"));
                return spl::success();
            }
            set_status(spl::network::status::connected);
            logger::info("Connection [{}] Connection reestablished", id());
            return spl::success();
        }

        [[nodiscard]] constexpr auto disconnect() noexcept -> result<void> {
            logger::info("Connection [{}] Trying to disconnect...", id());
            set_status(spl::network::status::disconnecting);
            set_status(spl::network::status::disconnected);
            logger::info("Connection [{}] Connection disconnected", id());
            return spl::success();
        }

        [[nodiscard]] constexpr auto close() noexcept -> result<void> {
            SPL_ASSERT_MSG(connection_, "trying to close an uninitialized connection");
            logger::info("Connection [{}] Trying to close the connection", id());
            set_status(spl::network::status::closing);
            err_return(connection_->close());
            set_status(spl::network::status::closed);
            logger::info("Connection [{}] Connection closed", id());
            return spl::success();
        }

        template <typename FunctorT>
        [[nodiscard]] constexpr auto poll(FunctorT&& functor) -> result<void> {
            if (not scheduler_.scheduled() or not scheduler_.poll(std::chrono::steady_clock::now())) [[unlikely]] {
                return spl::success();
            }
            return reconnect();
        }

        [[nodiscard]] constexpr auto bytes_readable() -> result<std::size_t> {
            SPL_ASSERT_MSG(connection_, "trying to read from an uninitialized connection");
            return connection_->bytes_readable();
        }

        template <typename MutableBufferSequenceT>
        [[nodiscard]] constexpr auto read(MutableBufferSequenceT&& buffer) -> result<std::size_t> {
            SPL_ASSERT_MSG(connection_, "trying to read from an uninitialized connection");
            if (scheduler_.scheduled()) [[unlikely]] {
                return 0;
            }

            auto const operation = connection_->bytes_readable();
            if (spl::failed(operation)) [[unlikely]] {
                err_return(intercept<std::size_t>(operation, "bytes_readable"));
                return 0;
            }

            constexpr auto minimum_buffer = 64;
            if (operation.value() <= minimum_buffer) [[unlikely]] {
                return 0;
            }

            network::error_code error_code;
            auto const bytes = connection_->read(std::forward<MutableBufferSequenceT>(buffer), error_code);
            return intercept<std::size_t>(error_code, "read", bytes);
        }

        template <typename ConstBufferSequenceT>
        [[nodiscard]] constexpr auto write(ConstBufferSequenceT&& buffer) -> result<std::size_t> {
            SPL_ASSERT_MSG(connection_, "trying to write to an uninitialized connection");
            if (scheduler_.scheduled()) [[unlikely]] {
                return 0;
            }

            network::error_code error_code;
            auto const bytes = connection_->write(std::forward<ConstBufferSequenceT>(buffer), error_code);
            return intercept<std::size_t>(error_code, "write", bytes);
        }

        [[nodiscard]] constexpr auto ping() noexcept -> result<void> {
            if constexpr (requires { connection_.ping(); }) {
                return connection_.ping();
            }
            return spl::success();
        }

        [[nodiscard]] constexpr auto status() const noexcept -> spl::network::status {
            return status_;
        }

        [[nodiscard]] constexpr auto ready() const noexcept -> bool {
            return status_ == spl::network::status::connected;
        }

        [[nodiscard]] constexpr auto scheduled() const noexcept -> bool {
            return scheduler_.scheduled();
        }

        [[nodiscard]] explicit constexpr operator bool() const noexcept {
            return ready();
        }

        [[nodiscard]] constexpr auto id() const noexcept -> spl::network::connection_id const& {
            return id_;
        }

        constexpr auto set_status(spl::network::status status) noexcept -> void {
            status_ = status;
        }

    private:
        template <typename AlternativeT, typename... ArgsT>
        [[nodiscard]] constexpr auto intercept(spl::network::error_code const& error, std::string_view operation,
                                               ArgsT&&... args) -> result<AlternativeT> {
            if (error.failed()) [[unlikely]] {
                err_return(intercept<void>(spl::failure(error), operation));
            }
            return AlternativeT{std::forward<ArgsT>(args)...};
        }

        template <typename AlternativeT>
        [[nodiscard]] constexpr auto intercept(result<AlternativeT> const& current, std::string_view operation)
            -> result<void> {
            logger::error("Connection [{}] Failed to perform operation [{}]: \"{}\"", id(), operation,
                          current.error().message().data());
            set_status(spl::network::status::disconnected);
            err_return(scheduler_.schedule(std::chrono::steady_clock::now()));
            return spl::success();
        }

        template <typename BufferT>
        [[nodiscard]] constexpr auto to_view(BufferT&& buffer, std::size_t length) noexcept -> std::string_view {
            if constexpr (requires { buffer.data().data(); }) {
                return std::string_view{reinterpret_cast<char const*>(buffer.data().data()), length};
            } else {
                return std::string_view{reinterpret_cast<char const*>(std::data(buffer)), length};
            }
        }

        router_type router_;
        spl::network::connection_id id_;
        spl::network::connector::scheduler scheduler_;
        spl::network::status status_{spl::network::status::uninitialized};
        std::optional<connection_type> connection_{std::nullopt};
        std::optional<endpoint> endpoint_{std::nullopt};
    };
} // namespace spl::network::connector