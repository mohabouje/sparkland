#pragma once

#include <concepts>
#include <tuple>

namespace spl::concepts {

    template <class T, std::size_t N>
    concept has_tuple_element = requires(T tuple) {
        typename std::tuple_element_t<N, std::remove_const_t<T>>;
        { get<N>(tuple) } -> std::convertible_to<std::tuple_element_t<N, T> const&>;
    };

    template <class T>
    concept tuple_like = !std::is_reference_v<T> && requires(T) {
        typename std::tuple_size<T>::type;
        requires std::derived_from<std::tuple_size<T>, std::integral_constant<std::size_t, std::tuple_size_v<T>>>;
    } && []<std::size_t... N>(std::index_sequence<N...>) {
        return (has_tuple_element<T, N> && ...);
    }(std::make_index_sequence<std::tuple_size_v<T>>());

    template <class T>
    concept tuple = tuple_like<T>;

} // namespace spl::concepts
