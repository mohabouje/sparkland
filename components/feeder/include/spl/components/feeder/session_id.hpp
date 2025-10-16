#pragma once

#include <string>
#include <format>

namespace spl::components::feeder {

    struct session_id {
        constexpr session_id(std::string initiator, std::string acceptor, std::size_t token_id = std::rand()) :
            initiator_(std::move(initiator)),
            acceptor_(std::move(acceptor)),
            token_id_(std::move(token_id)),
            id_(std::format("{}[{:^{}s}]", initiator_, acceptor_, 11)) {}

        [[nodiscard]] constexpr static auto make(std::string_view remote) noexcept -> session_id {
            return session_id("client", std::string(remote));
        }

        [[nodiscard]] constexpr auto operator==(session_id const& other) const noexcept -> bool {
            return initiator_ == other.initiator_ && acceptor_ == other.acceptor_ && token_id_ == other.token_id_;
        }

        [[nodiscard]] constexpr auto operator!=(session_id const& other) const noexcept -> bool {
            return not(*this == other);
        }

        [[nodiscard]] constexpr auto initiator() const noexcept -> std::string const& {
            return initiator_;
        }

        [[nodiscard]] constexpr auto acceptor() const noexcept -> std::string const& {
            return acceptor_;
        }

        [[nodiscard]] constexpr auto token_id() const noexcept -> std::size_t {
            return token_id_;
        }

        [[nodiscard]] constexpr auto connection_id() const noexcept -> std::string const& {
            return id_;
        }

    private:
        std::string initiator_;
        std::string acceptor_;
        std::size_t token_id_;
        std::string id_;
    };

} // namespace spl::components::feeder
