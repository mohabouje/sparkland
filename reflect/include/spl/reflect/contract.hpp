#pragma once

#include "spl/concepts/types.hpp"
#include "spl/concepts/tuple.hpp"
#include "spl/concepts/object.hpp"
#include "spl/concepts/cxxhash.hpp"
#include "spl/meta/typeinfo.hpp"

namespace spl::reflect {

    template <typename T, typename = void>
    struct identifier {
        using type                      = std::decay_t<T>;
        constexpr static auto unique_id = meta::type_name<type>();
        constexpr static auto hash      = concepts::xxhash::xxh32(unique_id);
    };

    template <typename TypeT>
    concept reflectable = concepts::object<TypeT> or concepts::scoped_enum<TypeT>;

} // namespace spl::reflect