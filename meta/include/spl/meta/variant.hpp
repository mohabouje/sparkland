#pragma once

#include "spl/core/assert.hpp"
#include "spl/concepts/variant.hpp"

#include <cstdlib>
#include <iosfwd>
#include <utility>
#include <variant>

namespace spl { inline namespace meta {

    namespace internal {

        template <std::size_t Index, typename FunctorT, typename VariantT>
        [[nodiscard]] constexpr auto visit_index_starting_at(FunctorT&& functor, VariantT& variant) {
            if (variant.index() == Index) {
                return functor(std::get<Index>(std::forward<VariantT>(variant)));
            }

            if constexpr (Index + 1 < std::variant_size_v<std::decay_t<VariantT>>) {
                return visit_index_starting_at<Index + 1>(std::forward<FunctorT>(functor),
                                                          std::forward<VariantT>(variant));
            }

            std::unreachable();
        }

        [[gnu::cold, gnu::noinline, noreturn]] inline static void throw_bad_variant_access() {
#ifdef __EXCEPTIONS
            throw std::bad_variant_access();
#else
            std::abort();
#endif
        }

        template <typename... WhateverT>
        struct joined;

        template <typename... TypesT, typename... OthersT>
        struct joined<std::variant<TypesT...>, OthersT...> {
            using type = std::variant<TypesT..., OthersT...>;
        };

        template <typename... WhateverT>
        struct prefix;

        template <typename... TypesT, typename... OthersT>
        struct prefix<std::variant<TypesT...>, OthersT...> {
            using type = std::variant<OthersT..., TypesT...>;
        };

        template <typename... Ts>
        struct smallest_type;

        template <typename EventT>
        struct smallest_type<EventT> {
            using type = EventT;
        };

        template <typename T, typename U, typename... Rest>
        struct smallest_type<T, U, Rest...> {
            using type = typename smallest_type<std::conditional_t<(sizeof(T) <= sizeof(U)), T, U>, Rest...>::type;
        };

        template <typename... Ts>
        using smallest_type_t = typename smallest_type<Ts...>::type;

    } // namespace internal

    template <typename FunctorT, typename VariantT>
    [[nodiscard]] constexpr auto unsafe_fast_visit(FunctorT&& functor, VariantT& variant) {
        SPL_ASSERT_MSG(!variant.valueless_by_exception(), "assumes variant is not valueless");
        return internal::visit_index_starting_at<0>(std::forward<FunctorT>(functor), std::forward<VariantT>(variant));
    }

    template <typename FunctorT, typename VariantT>
    [[nodiscard]] constexpr auto fast_visit(FunctorT&& functor, VariantT& variant) {
        if (variant.valueless_by_exception()) [[unlikely]] {
            internal::throw_bad_variant_access();
        }
        return internal::visit_index_starting_at<0>(std::forward<FunctorT>(functor), std::forward<VariantT>(variant));
    }

    template <class... Ts>
    struct overload : Ts... {
        using Ts::operator()...;
    };

    template <class... Ts>
    overload(Ts...) -> overload<Ts...>;

    template <class... FunctorsT, typename VariantT>
    [[nodiscard]] constexpr auto fast_overload_visit(VariantT& variant, FunctorsT&&... functors) {
        return fast_visit(overload{std::forward<FunctorsT>(functors)...}, std::forward<VariantT>(variant));
    }

    template <class... FunctorsT, typename VariantT>
    [[nodiscard]] constexpr auto unsafe_fast_overload_visit(VariantT& variant, FunctorsT&&... functors) {
        return unsafe_fast_visit(overload{std::forward<FunctorsT>(functors)...}, std::forward<VariantT>(variant));
    }

    template <typename T>
    struct is_variant : std::false_type {};

    template <typename... Args>
    struct is_variant<std::variant<Args...>> : std::true_type {};

    template <typename T>
    inline constexpr bool is_variant_v = is_variant<T>::value;

    template <typename... LeftT, typename... RightT>
    [[nodiscard]] constexpr auto compare(std::variant<LeftT...> const& left,
                                         std::variant<RightT...> const& right) noexcept -> bool {
        return std::visit(
            [&right]<typename CandidateLeftT>(CandidateLeftT&& candidate_left) -> bool {
                using left_type = std::decay_t<CandidateLeftT>;
                return std::visit(
                    [&candidate_left]<typename CandidateRightT>(CandidateRightT&& candidate_right) -> bool {
                        using right_type = std::decay_t<CandidateRightT>;
                        if constexpr (std::is_same_v<right_type, left_type>) {
                            return candidate_left == candidate_right;
                        }
                        return false;
                    },
                    right);
            },
            left);
    }

    template <typename T, typename Variant, std::size_t Index = 0, typename = void>
    struct is_variant_member : std::false_type {};

    template <typename T, typename... Types, std::size_t Index>
    struct is_variant_member<T, std::variant<Types...>, Index, std::enable_if_t<(Index < sizeof...(Types))>> {
        using CurrentType = std::variant_alternative_t<Index, std::variant<Types...>>;
        constexpr static auto value =
            std::is_same_v<T, CurrentType> ? true : is_variant_member<T, std::variant<Types...>, Index + 1>::value;
    };

    template <typename T, typename Variant>
    constexpr auto is_variant_member_v = is_variant_member<T, Variant>::value;

    template <typename VariantT, typename T, std::size_t IndexV = 0>
    requires is_variant_v<VariantT>
    [[nodiscard]] constexpr auto variant_index() noexcept -> std::size_t {
        static_assert(std::variant_size_v<VariantT> > IndexV, "Type not found in variant");
        if constexpr (IndexV == std::variant_size_v<VariantT>) {
            return IndexV;
        } else if constexpr (std::is_same_v<std::variant_alternative_t<IndexV, VariantT>, T>) {
            return IndexV;
        } else {
            return variant_index<VariantT, T, IndexV + 1>();
        }
    }

    template <typename VariantT, typename... OthersT>
    using variant_join = typename internal::joined<VariantT, OthersT...>::type;

    template <typename VariantT, typename... OthersT>
    using variant_prefix = typename internal::prefix<VariantT, OthersT...>::type;

    template <class V>
    struct smallest_variant;

    template <class... Ts>
    struct smallest_variant<std::variant<Ts...>> {
        using type = internal::smallest_type_t<Ts...>;
    };

    template <typename V>
    using smallest_variant_t = typename smallest_variant<V>::type;

}} // namespace spl::meta
