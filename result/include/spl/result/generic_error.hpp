#pragma once

#include "spl/result/error_message.hpp"

#include <boost/outcome/experimental/status_result.hpp>

// Based on https://github.com/boostorg/outcome/blob/master/test/tests/experimental-p0709a.cpp

namespace spl { inline namespace outcome {

    class _generic_error_domain;
    using generic_error = boost::outcome_v2::experimental::status_code<_generic_error_domain>;

    class _generic_error_domain final : public boost::outcome_v2::experimental::status_code_domain {
        template <class DomainType>
        friend class status_code;
        using base = status_code_domain;

    public:
        using string_ref     = string_ref;
        using unique_id_type = unique_id_type;
        using value_type     = error_message;

        constexpr static auto domain_name = "generic_domain";
        constexpr static auto domain_uuid = 0x230f170194fcc6c7;

        constexpr explicit _generic_error_domain() noexcept : base(domain_uuid) {}
        constexpr _generic_error_domain(_generic_error_domain const&)                    = default;
        constexpr _generic_error_domain(_generic_error_domain&&)                         = default;
        constexpr auto operator=(_generic_error_domain const&) -> _generic_error_domain& = default;
        constexpr auto operator=(_generic_error_domain&&) -> _generic_error_domain&      = default;
        constexpr ~_generic_error_domain()                                               = default;

        constexpr static auto get() -> _generic_error_domain const&;

        [[nodiscard]] constexpr auto name() const noexcept -> base::string_ref final {
            static auto const ALLOCATED = base::string_ref(domain_name);
            return ALLOCATED;
        }

        [[nodiscard]] constexpr auto
            _do_failure(boost::outcome_v2::experimental::status_code<void> const& code) const noexcept -> bool final {
            assert(code.domain() == *this);
            auto const& underlying = static_cast<generic_error const&>(code);
            return not std::empty(underlying.value().what());
        }

        [[nodiscard]] constexpr auto
            _do_equivalent(boost::outcome_v2::experimental::status_code<void> const& code1,
                           boost::outcome_v2::experimental::status_code<void> const& code2) const noexcept
            -> bool override {
            assert(code1.domain() == *this);
            auto const& underlying_1 = static_cast<generic_error const&>(code1);
            auto const& underlying_2 = static_cast<generic_error const&>(code2);
            return underlying_1.value().what() == underlying_2.value().what();
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
            auto const& underlying = static_cast<generic_error const&>(code);
            auto const& data       = underlying.value().what();
            auto* p                = static_cast<char*>(malloc(std::size(data)));
            std::copy(std::cbegin(data), std::cend(data), p);
            return atomic_refcounted_string_ref(p, std::size(data));
        }

        constexpr auto _do_throw_exception(boost::outcome_v2::experimental::status_code<void> const& code) const
            -> void override {
            assert(code.domain() == *this);
            auto const& underlying = static_cast<generic_error const&>(code);
            throw std::runtime_error(underlying.value().what().data());
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

    inline constexpr _generic_error_domain generic_error_domain;
    constexpr auto _generic_error_domain::get() -> _generic_error_domain const& {
        return generic_error_domain;
    }

}} // namespace spl::outcome

template <typename ErrorMessageT>
requires(std::is_same_v<std::decay_t<ErrorMessageT>, spl::outcome::error_message>)
[[nodiscard]] constexpr auto make_status_code(ErrorMessageT&& e) -> spl::generic_error {
    return spl::generic_error{boost::outcome_v2::experimental::in_place, std::forward<ErrorMessageT>(e)};
}

template <typename... ArgsT>
[[nodiscard]] constexpr auto make_status_code(std::format_string<ArgsT...> const& format, ArgsT&&... args)
    -> spl::generic_error {
    return make_status_code(spl::outcome::error_message{format, std::forward<ArgsT>(args)...});
}

static_assert(std::is_default_constructible_v<spl::outcome::generic_error>);
static_assert(std::is_nothrow_default_constructible_v<spl::outcome::generic_error>);
static_assert(std::is_copy_constructible_v<spl::outcome::generic_error>);
// static_assert(std::is_trivially_copy_constructible_v<spl::outcome::generic_error>);
static_assert(std::is_nothrow_copy_constructible_v<spl::outcome::generic_error>);
static_assert(std::is_copy_assignable_v<spl::outcome::generic_error>);
static_assert(std::is_trivially_copy_assignable_v<spl::outcome::generic_error>);
static_assert(std::is_nothrow_copy_assignable_v<spl::outcome::generic_error>);
static_assert(std::is_trivially_destructible_v<spl::outcome::generic_error>);
static_assert(std::is_nothrow_destructible_v<spl::outcome::generic_error>);