#pragma once

#include <array>
#include <format>

namespace spl { inline namespace outcome {

    class error_message {
    public:
        inline constexpr static auto maximum_error_length = 512;

        constexpr error_message() noexcept                                        = default;
        constexpr error_message(error_message const&) noexcept                    = default;
        constexpr error_message(error_message&&) noexcept                         = default;
        constexpr auto operator=(error_message const&) noexcept -> error_message& = default;
        constexpr auto operator=(error_message&&) noexcept -> error_message&      = default;

        template <typename... ArgsT>
        constexpr explicit error_message(std::format_string<ArgsT...> const& format, ArgsT&&... args) noexcept {
            std::ranges::fill(buffer_, '\0');
            std::format_to_n(std::data(buffer_), maximum_error_length, format,
                             std::forward<ArgsT>(args)...);
        }

        constexpr explicit error_message(std::exception const& exception) noexcept :
            error_message("{}", exception.what()) {}

        [[nodiscard]] constexpr auto what() const noexcept -> std::string_view {
            return {std::data(buffer_)};
        }

    private:
        std::array<char, maximum_error_length> buffer_{};
    };

}} // namespace spl::outcome

static_assert(std::is_default_constructible_v<spl::outcome::error_message>);
static_assert(std::is_nothrow_default_constructible_v<spl::outcome::error_message>);
static_assert(std::is_copy_constructible_v<spl::outcome::error_message>);
static_assert(std::is_trivially_copy_constructible_v<spl::outcome::error_message>);
static_assert(std::is_nothrow_copy_constructible_v<spl::outcome::error_message>);
static_assert(std::is_copy_assignable_v<spl::outcome::error_message>);
static_assert(std::is_trivially_copy_assignable_v<spl::outcome::error_message>);
static_assert(std::is_nothrow_copy_assignable_v<spl::outcome::error_message>);
static_assert(std::is_trivially_destructible_v<spl::outcome::error_message>);
static_assert(std::is_nothrow_destructible_v<spl::outcome::error_message>);