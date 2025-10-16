#pragma once

#include "spl/concepts/tuple.hpp"
#include "spl/concepts/types.hpp"
#include "spl/concepts/optional.hpp"
#include "spl/concepts/variant.hpp"

#include <type_traits>
#include <concepts>
#include <cstddef>

namespace spl::concepts {

    template <typename ObjectT>
    concept object =
        std::is_aggregate_v<std::decay_t<ObjectT>> and (not std::ranges::range<std::decay_t<ObjectT>>) and
        (not concepts::optional<std::decay_t<ObjectT>>) and (not concepts::tuple_like<std::decay_t<ObjectT>>) and
        (not concepts::variant<std::decay_t<ObjectT>>);

} // namespace spl::concepts
