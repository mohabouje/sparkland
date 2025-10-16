#pragma once

#include "spl/reflect/contract.hpp"
#include "spl/concepts/variant.hpp"
#include "spl/concepts/cxxhash.hpp"
#include "spl/codec/json/tagger.hpp"
#include "spl/meta/variant.hpp"

namespace spl::components::feeder {

    enum class tagger_type { json };

    template <tagger_type TypeV, typename ObjectT>
    struct tagger;

    template <typename AnyT>
    struct tagger<tagger_type::json, AnyT> : public codec::json::tagger {
        template <char... Tag>
        [[nodiscard]] static auto header(std::string_view value, std::span<char> buffer) -> result<std::size_t> {
            return 0;
        }

        template <char... Tag>
        [[nodiscard]] static auto footer(std::string_view value, std::span<char> buffer) -> result<std::size_t> {
            using base_type    = codec::json::tagger;
            auto const resized = base_type::set_tag<Tag...>(value, buffer);
            auto const extra   = std::size(resized) - std::size(buffer);
            return extra;
        }

        template <char... Tag, typename BufferT>
        [[nodiscard]] static auto hash(BufferT&& input) noexcept -> std::pair<std::string_view, std::size_t> {
            using base_type       = codec::json::tagger;
            auto const unique_tag = base_type::get_tag<Tag...>(input);
            SPL_ASSERT_MSG(not std::empty(unique_tag), "failed to find tag");
            return {unique_tag, 0};
        }

        template <typename ObjectT>
        [[nodiscard]] constexpr static auto inbound() noexcept -> std::uint64_t {
            return spl::reflect::identifier<ObjectT>::hash;
        }

        template <typename ObjectT>
        [[nodiscard]] constexpr static auto outbound() noexcept -> decltype(auto) {
            return spl::reflect::identifier<ObjectT>::unique_id;
        }

        template <char... Tag>
        [[nodiscard]] constexpr static auto starts_with(std::string_view input) noexcept -> bool {
            return [&]<std::size_t... I>(std::index_sequence<I...>) {
                return std::size(input) >= sizeof...(Tag) and ((input[I] == Tag) and ...);
            }(std::make_index_sequence<sizeof...(Tag)>{});
        }

        [[nodiscard]] constexpr static auto strip(std::string_view input) noexcept -> std::string_view {
            auto const filter        = [](auto value) { return value != ' ' and value != '"'; };
            auto const front_iter    = std::find_if(std::begin(input), std::end(input), filter);
            auto const front_padding = std::distance(std::begin(input), front_iter);
            auto const back_iter     = std::find_if(std::rbegin(input), std::rend(input), filter);
            auto const back_padding  = std::distance(std::rbegin(input), back_iter);
            return input.substr(front_padding, std::size(input) - front_padding - back_padding);
        }
    };

    template <typename ObjectT>
    using json_tagger = tagger<tagger_type::json, ObjectT>;

} // namespace spl::components::feeder