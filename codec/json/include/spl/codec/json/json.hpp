#pragma once

#include "spl/codec/json/contract/contract.hpp"
#include "spl/result/result.hpp"
#include "spl/logger/logger.hpp"

#include <algorithm>
#include <optional>
#include <variant>
#include <filesystem>
#include <fstream>

namespace spl::codec::json {

    template <spl::concepts::string StringT, typename TypeT, typename... ArgsT>
    [[nodiscard]] constexpr auto from_json(StringT&& input, TypeT& output, ArgsT&&... args) noexcept
        -> spl::result<void> {
        auto const view = std::string_view{std::data(input), std::size(input)};
        return spl::safe_call(
            [&]() { output = daw::json::from_json<std::decay_t<TypeT>>(view, std::forward<ArgsT>(args)...); });
    }

    template <typename TypeT, spl::concepts::string StringT>
    [[nodiscard]] constexpr auto to_json(TypeT&& input, StringT&& output) noexcept -> spl::result<std::size_t> {
        return spl::safe_call([&]() -> std::size_t {
            auto const iter  = daw::json::to_json(input, std::begin(output));
            auto const bytes = std::distance(std::begin(output), iter);
            return bytes;
        });
    }

    template <typename TypeT, spl::concepts::string StringT, typename... ArgsT>
    [[nodiscard]] constexpr auto from_json(StringT&& input, ArgsT&&... args) noexcept -> spl::result<TypeT> {
        TypeT output;
        err_return(from_json(std::forward<StringT>(input), output, std::forward<ArgsT>(args)...));
        return output;
    }

    template <spl::concepts::string StringT, typename TypeT>
    [[nodiscard]] constexpr auto to_json(TypeT&& input) noexcept -> spl::result<StringT> {
        auto output = std::array<char, 1 << 16>{};
        auto view   = std::span<char>{std::data(output), std::size(output)};
        auto bytes  = err_return(to_json(std::forward<TypeT>(input), view));
        return StringT{std::data(output), bytes};
    }

    template <typename TypeT>
    [[nodiscard]] constexpr auto to_json(std::filesystem::path const& path, TypeT&& input) noexcept
        -> spl::result<void> {
        std::filesystem::create_directories(path.parent_path());
        auto output = std::array<char, 1 << 16>{};
        auto view   = std::span<char>{std::data(output), std::size(output)};
        auto bytes  = err_return(to_json(std::forward<TypeT>(input), view));
        auto stream = std::ofstream{path};
        stream << std::string_view{std::data(output), bytes};
        return spl::success();
    }

    template <spl::concepts::string StringT, typename ExpectedT, typename... TypeT>
    [[nodiscard]] constexpr auto try_from_json(StringT&& input, std::variant<TypeT...>& output) noexcept
        -> spl::result<void> {
        output = err_return(from_json<ExpectedT>(std::forward<StringT>(input)));
        return success();
    }

    template <spl::concepts::string StringT, typename... TypeT>
    [[nodiscard]] constexpr auto from_json(StringT&& input, std::variant<TypeT...>& output) noexcept
        -> spl::result<void> {
        auto const result = std::array{try_from_json<TypeT>(input, output)...};
        if (std::any_of(result.begin(), result.end(), [](auto const& result) { return spl::succeeded(result); })) {
            return success();
        }

        logger::error("Failed to parse variant type for input: \"{}\"", input);
        [&]<std::size_t... IndexV>(std::index_sequence<IndexV...>) {
            auto dispatch = [&]<typename EventT>(std::size_t index, std::optional<EventT>) {
                if (spl::failed(result[index])) {
                    spl::logger::error("Candidate {} error: {}", index, result[index].error().message().data());
                }
            };
            (dispatch(IndexV, std::optional<std::variant_alternative_t<IndexV, std::variant<TypeT...>>>(std::nullopt)),
             ...);
        }(std::make_index_sequence<sizeof...(TypeT)>{});

        return spl::failure("Failed to parse variant type, check the logs for more information");
    }

} // namespace spl::codec::json