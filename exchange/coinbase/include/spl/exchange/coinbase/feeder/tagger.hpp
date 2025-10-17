#pragma once

#include "spl/components/feeder/tagger.hpp"

namespace spl::exchange::coinbase::feeder {

    template <typename ObjectT>
    struct tagger : public spl::components::feeder::json_tagger<ObjectT> {
        template <char... Tag, typename BufferT>
        [[nodiscard]] static auto hash(BufferT&& input) noexcept -> std::pair<std::string_view, std::size_t> {
            using base_type    = spl::components::feeder::json_tagger<ObjectT>;
            auto const primary = base_type::template get_tag<Tag...>(input);
            if (not std::empty(primary)) [[unlikely]] {
                return {primary, 0};
            }
            return {"unknown", 0};
        }
    };

} // namespace spl::exchange::coinbase::feeder
