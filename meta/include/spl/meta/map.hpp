#pragma once

#include "spl/meta/typed.hpp"

#include <boost/mp11/map.hpp>

namespace spl { inline namespace meta {

    template <typename... PairT>
    using map = boost::mp11::mp_list<PairT...>;

    template <auto KeyV, typename ValueT>
    using vpair = std::pair<meta::typed<KeyV>, ValueT>;

    template <typename MapT, typename KeyT>
    using map_at = typename boost::mp11::mp_map_find<MapT, KeyT>::second_type;

    template <typename MapT, typename KeyT>
    using map_contains = boost::mp11::mp_map_contains<MapT, KeyT>;

    template <typename MapT>
    using keys = boost::mp11::mp_transform<boost::mp11::mp_first, MapT>;

    template <typename MapT, typename KeyT>
    constexpr auto map_contains_v = map_contains<MapT, KeyT>::value;

}} // namespace spl::meta