#pragma once

#include "spl/meta/typed.hpp"

#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>

#include <tuple>
#include <variant>

namespace spl { inline namespace meta {
    namespace internal {

        template <typename T>
        struct dispatcher;

        template <auto... ValuesV>
        struct dispatcher<boost::mp11::mp_list<meta::typed<ValuesV>...>> {
            inline constexpr static auto values = std::array{ValuesV...};
        };

    } // namespace internal

    template <typename... TypeT>
    using list = boost::mp11::mp_list<TypeT...>;

    template <auto... ValuesV>
    using vlist = meta::list<meta::typed<ValuesV>...>;

    template <typename... ListT>
    using joined = typename boost::mp11::mp_append<ListT...>;

    template <typename... ListT>
    using intersection = typename boost::mp11::mp_set_intersection<ListT...>;

    template <typename ListT>
    using unique = typename boost::mp11::mp_unique<ListT>;

    template <typename ListT, typename ValueT>
    using contains = boost::mp11::mp_contains<ListT, ValueT>;

    template <typename ListT, std::size_t IndexV>
    using list_at = boost::mp11::mp_at_c<ListT, IndexV>;

    template <typename ListT, typename ValueT>
    constexpr auto contains_v = contains<ListT, ValueT>::value;

    template <typename ValueListT>
    inline constexpr auto to_values = internal::dispatcher<ValueListT>::values;

    template <typename ListT, typename TypeT>
    [[nodiscard]] constexpr auto index_for() noexcept -> std::size_t {
        using iter_type = boost::mp11::mp_find<ListT, TypeT>;
        return boost::mp11::mp_size<boost::mp11::mp_take<ListT, iter_type>>::value;
    }

    template <typename ListT>
    using as_tuple = boost::mp11::mp_rename<ListT, std::tuple>;

    template <typename ListT>
    using as_variant = boost::mp11::mp_rename<ListT, std::variant>;

}} // namespace spl::meta