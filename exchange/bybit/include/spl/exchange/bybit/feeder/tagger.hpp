#pragma once

#include "spl/components/feeder/tagger.hpp"

namespace spl::exchange::bybit::feeder {

    template <typename ObjectT>
    struct tagger : public spl::components::feeder::json_tagger<ObjectT> {
        template <char... Tag, typename BufferT>
        [[nodiscard]] static auto hash(BufferT&& input) noexcept -> std::pair<std::string_view, std::size_t> {
            using base_type    = spl::components::feeder::json_tagger<ObjectT>;
            auto const primary = base_type::template get_tag<Tag...>(input);
            if (not std::empty(primary)) [[unlikely]] {
                return {primary, 0};
            }

            auto const label = [&]() {
                auto const topic = base_type::template get_tag<'t', 'o', 'p', 'i', 'c'>(input);
                if (std::empty(topic)) [[unlikely]] {
                    return "unknown";
                }
                switch (topic[0]) {
                    case 'o': {
                        if (auto const level1 = topic[10] == '1'; level1) [[unlikely]] {
                            return "orderbook:level1:snapshot";
                        }
                        constexpr auto alternatives = std::array{"orderbook:level2:delta", "orderbook:level2:snapshot"};
                        auto const type             = base_type::template get_tag<'t', 'y', 'p', 'e'>(input);
                        return alternatives[type[0] == 's'];
                    }
                    case 'p':
                        return "trade:snapshot";
                    case 'k':
                        return "kline:snapshot";
                    case 't':
                        return "ticker:snapshot";
                    default:
                        return "unknown";
                }
            }();
            return {label, 0};
        }
    };

} // namespace spl::exchange::bybit::feeder
