#pragma once

#include "spl/reflect/contract.hpp"
#include "spl/reflect/object.hpp"

template <spl::concepts::object ObjectT>
constexpr auto operator==(ObjectT const& lhs, ObjectT const& rhs) {
    return [&]<std::size_t... IndexV>(std::index_sequence<IndexV...>) {
        return ((boost::pfr::get<IndexV>(lhs) == boost::pfr::get<IndexV>(rhs)) && ...);
    }(std::make_index_sequence<boost::pfr::tuple_size_v<std::decay_t<ObjectT>>>());
}