#pragma once

#include "spl/reflect/contract.hpp"
#include "spl/concepts/types.hpp"
#include "spl/meta/tstring.hpp"

#include <boost/pfr.hpp>

#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace spl::reflect {

    namespace internal {

        template <std::size_t I, class T>
        constexpr auto get_name() noexcept {
            return boost::pfr::detail::stored_name_of_field<T, I>;
        }

    } // namespace internal

    template <concepts::object ObjectT>
    [[nodiscard]] constexpr auto size() noexcept -> std::size_t {
        return boost::pfr::tuple_size_v<std::decay_t<ObjectT>>;
    }

    template <concepts::object ObjectT>
    [[nodiscard]] constexpr auto count() noexcept -> std::size_t {
        return boost::pfr::tuple_size_v<std::decay_t<ObjectT>>;
    }

    template <concepts::object ObjectT>
    [[nodiscard]] constexpr auto names() noexcept {
        return []<std::size_t... IndexV>(std::index_sequence<IndexV...>&&) {
            return std::array{std::string_view(internal::get_name<IndexV, ObjectT>().data())...};
        }(std::make_index_sequence<boost::pfr::tuple_size_v<std::decay_t<ObjectT>>>());
    }

    template <std::size_t IndexV, concepts::object ObjectT>
    [[nodiscard]] consteval auto field_name() noexcept {
        return meta::tstring{internal::get_name<IndexV, ObjectT>()};
    }

    template <std::size_t IndexV, concepts::object ObjectT>
    using field_type = std::decay_t<decltype(boost::pfr::get<IndexV>(std::declval<ObjectT>()))>;

    template <std::size_t IndexV, concepts::object ObjectT>
    [[nodiscard]] constexpr auto get(ObjectT&& object) noexcept -> decltype(auto) {
        return boost::pfr::get<IndexV>(std::forward<ObjectT>(object));
    }

    template <concepts::object ObjectT>
    [[nodiscard]] constexpr auto forward_as_tuple(ObjectT&& object) -> decltype(auto) {
        return []<typename TypeT, std::size_t... IndexV>(TypeT&& object, std::index_sequence<IndexV...>&&) {
            return std::make_tuple(boost::pfr::get<IndexV>(object)...);
        }(std::forward<ObjectT>(object), std::make_index_sequence<boost::pfr::tuple_size_v<std::decay_t<ObjectT>>>());
    }

    template <concepts::object ObjectT, typename FunctorT>
    constexpr auto for_each(ObjectT&& object, FunctorT&& functor) noexcept(std::is_nothrow_invocable_v<FunctorT>)
        -> void {
        return []<std::size_t... IndexV>(ObjectT&& object, FunctorT&& functor, std::index_sequence<IndexV...>&&) {
            using value_type = std::decay_t<ObjectT>;
            (functor(static_cast<std::string_view>(field_name<IndexV, value_type>()),
                     boost::pfr::get<IndexV>(std::forward<ObjectT>(object))),
             ...);
        }(std::forward<ObjectT>(object), std::forward<FunctorT>(functor),
               std::make_index_sequence<size<std::decay_t<ObjectT>>()>());
    }

    template <concepts::object ObjectT, typename FunctorT>
    constexpr auto traverse_with_unique(FunctorT&& functor) noexcept(std::is_nothrow_invocable_v<FunctorT>) -> void {
        return []<std::size_t... IndexV>(FunctorT&& functor, std::index_sequence<IndexV...>&&) {
            using value_type = std::decay_t<ObjectT>;
            (functor(
                 static_cast<std::string_view>(field_name<IndexV, value_type>()),
                 std::unique_ptr<std::decay_t<decltype(boost::pfr::get<IndexV>(std::declval<ObjectT>()))>>(nullptr)),
             ...);
        }(std::forward<FunctorT>(functor), std::make_index_sequence<size<std::decay_t<ObjectT>>()>());
    }

    template <concepts::object ObjectT, typename FunctorT>
    constexpr auto traverse_with_optional(FunctorT&& functor) noexcept(std::is_nothrow_invocable_v<FunctorT>) -> void {
        return []<std::size_t... IndexV>(FunctorT&& functor, std::index_sequence<IndexV...>&&) {
            using value_type = std::decay_t<ObjectT>;
            (functor(
                 static_cast<std::string_view>(field_name<IndexV, value_type>()),
                 std::optional<std::decay_t<decltype(boost::pfr::get<IndexV>(std::declval<ObjectT>()))>>(std::nullopt)),
             ...);
        }(std::forward<FunctorT>(functor), std::make_index_sequence<size<std::decay_t<ObjectT>>()>());
    }

} // namespace spl::reflect
