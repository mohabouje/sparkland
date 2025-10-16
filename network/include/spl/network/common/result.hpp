#pragma once

#include "spl/result/result.hpp"
#include "spl/network/common/error_code.hpp"

namespace spl::network {

    class _network_domain;
    using network_error = boost::outcome_v2::experimental::status_code<_network_domain>;

    class _network_domain final : public boost::outcome_v2::experimental::status_code_domain {
        template <class DomainType>
        friend class status_code;
        using base = status_code_domain;

    public:
        using string_ref     = string_ref;
        using unique_id_type = unique_id_type;
        using value_type     = network::error_code;

        constexpr static auto domain_name = "network-domain";
        constexpr static auto domain_uuid = 0x230f170194fcc6c8;

        constexpr explicit _network_domain() noexcept : base(domain_uuid) {}
        constexpr _network_domain(_network_domain const&)                    = default;
        constexpr _network_domain(_network_domain&&)                         = default;
        constexpr auto operator=(_network_domain const&) -> _network_domain& = default;
        constexpr auto operator=(_network_domain&&) -> _network_domain&      = default;
        constexpr ~_network_domain()                                         = default;

        constexpr static auto get() -> _network_domain const&;

        [[nodiscard]] constexpr auto name() const noexcept -> base::string_ref final {
            static auto const VALUE = base::string_ref(domain_name);
            return VALUE;
        }

        [[nodiscard]] constexpr auto
            _do_failure(boost::outcome_v2::experimental::status_code<void> const& code) const noexcept -> bool final {
            assert(code.domain() == *this);
            auto const& underlying = static_cast<network_error const&>(code);
            return underlying.value().value() != 0;
        }

        [[nodiscard]] constexpr auto
            _do_equivalent(boost::outcome_v2::experimental::status_code<void> const& code1,
                           boost::outcome_v2::experimental::status_code<void> const& code2) const noexcept
            -> bool override {
            assert(code1.domain() == *this);
            auto const& underlying_1 = static_cast<network_error const&>(code1);
            auto const& underlying_2 = static_cast<network_error const&>(code2);
            return underlying_1.value().value() == underlying_2.value().value();
        }

        [[nodiscard]] constexpr auto
            _generic_code(boost::outcome_v2::experimental::status_code<void> const& code) const noexcept
            -> boost::outcome_v2::experimental::generic_code override {
            assert(code.domain() == *this);
            return {};
        }

        [[nodiscard]] constexpr auto
            _do_message(boost::outcome_v2::experimental::status_code<void> const& code) const noexcept
            -> base::string_ref override {
            assert(code.domain() == *this);
            auto const& underlying = static_cast<network_error const&>(code);
            auto const& data       = underlying.value().message();
            auto* p                = static_cast<char*>(malloc(std::size(data)));
            std::copy(std::cbegin(data), std::cend(data), p);
            return atomic_refcounted_string_ref(p, std::size(data));
        }

        constexpr auto _do_throw_exception(boost::outcome_v2::experimental::status_code<void> const& code) const
            -> void override {
            assert(code.domain() == *this);
            auto const& underlying = static_cast<network_error const&>(code);
            throw std::runtime_error(underlying.value().message());
        }

        [[nodiscard]] constexpr auto payload_info() const noexcept -> payload_info_t override {
            constexpr auto payload_size    = sizeof(value_type);
            constexpr auto total_size      = sizeof(status_code_domain*) + sizeof(value_type);
            constexpr auto total_alignment = (alignof(value_type) > alignof(status_code_domain*))
                                                 ? alignof(value_type)
                                                 : alignof(status_code_domain*);
            return {payload_size, total_size, total_alignment};
        }
    };

    constexpr _network_domain network_error_domain;
    constexpr auto _network_domain::get() -> _network_domain const& {
        return network_error_domain;
    }

    template <typename SomethingT>
    using result = result<SomethingT, network_error>;

} // namespace spl::network

template <typename ErrorMessageT>
requires(std::is_same_v<std::decay_t<ErrorMessageT>, boost::system::error_code>)
[[nodiscard]] constexpr auto make_status_code(ErrorMessageT&& e) -> spl::network::network_error {
    return spl::network::network_error{boost::outcome_v2::experimental::in_place, std::forward<ErrorMessageT>(e)};
}

template <class T, class U>
requires std::is_constructible_v<T, U>
struct boost::outcome_v2::convert::value_or_error<spl::outcome::result<T>, spl::network::result<U>> {
    constexpr static bool enable_result_inputs  = true;
    constexpr static bool enable_outcome_inputs = false;

    template <typename TypeT>
    requires(std::is_same_v<spl::network::result<U>, std::decay_t<TypeT>>)
    constexpr auto operator()(TypeT&& src) noexcept -> spl::outcome::result<T> {
        if (not src.has_value()) [[unlikely]] {
            return spl::failure(spl::error_message("{}", std::forward<TypeT>(src).error().value().message()));
        }
        return success(std::forward<TypeT>(src).value());
    }
}; // namespace boost::outcome_v2::convert