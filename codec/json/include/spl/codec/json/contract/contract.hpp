#pragma once

#include "spl/codec/json/contract/unnamed.hpp"
#include "spl/codec/json/contract/named.hpp"
#include "spl/concepts/object.hpp"
#include "spl/meta/variant.hpp"
#include "spl/concepts/types.hpp"
#include "spl/reflect/contract.hpp"
#include "spl/reflect/object.hpp"
#include "spl/types/decimal.hpp"
#include "spl/concepts/types.hpp"

#include <cstdint>
#include <memory>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>

namespace spl::codec::json::contract {

    template <typename>
    struct member_list;

    template <spl::concepts::object ObjectT, std::size_t... IndexV>
    struct member_list<std::pair<ObjectT, std::index_sequence<IndexV...>>> {
        using value_type = std::decay_t<ObjectT>;
        using type =
            daw::json::json_member_list<typename named_mapper<reflect::field_name<IndexV, value_type>(),
                                                              reflect::field_type<IndexV, value_type>>::type...>;
    };

    template <spl::concepts::object ObjectT>
    struct member_list<ObjectT> {
        using value_type   = std::decay_t<ObjectT>;
        using indexer_type = typename std::decay_t<decltype(std::make_index_sequence<reflect::size<ObjectT>()>())>;
        using type         = typename member_list<std::pair<ObjectT, indexer_type>>::type;
    };

} // namespace spl::codec::json::contract

namespace daw::json {

    template <spl::concepts::object ObjectT>
    struct json_data_contract<ObjectT> {
        using type = typename spl::codec::json::contract::member_list<std::decay_t<ObjectT>>::type;

        template <typename InstanceT>
        constexpr static auto to_json_data(InstanceT&& value) noexcept {
            return spl::reflect::forward_as_tuple(std::forward<InstanceT>(value));
        }
    };

    template <spl::concepts::scoped_enum EnumT>
    struct json_data_contract<EnumT> {
        using type = json_type_alias<std::underlying_type_t<EnumT>>;

        template <typename InstanceT>
        constexpr static auto to_json_data(InstanceT&& v) {
            return std::to_underlying(v);
        }
    };

    template <typename RepT, typename AnotherT>
    struct json_data_contract<std::chrono::duration<RepT, AnotherT>> {
        using type = json_type_alias<RepT>;

        constexpr static auto to_json_data(std::chrono::duration<RepT, AnotherT> const& v) {
            return v.count();
        }
    };

    template <typename... TypeT>
    struct json_data_contract<std::tuple<TypeT...>> {
        using type = json_tuple_member_list<TypeT...>;

        constexpr static auto to_json_data(std::tuple<TypeT...> const& data) {
            return data;
        }
    };

    template <std::int16_t PrecisionV, spl::concepts::strictly_integral MantissaT>
    struct json_data_contract<spl::types::decimal<PrecisionV, MantissaT>> {
        using type = json_type_alias<double>;

        constexpr static auto to_json_data(spl::types::decimal<PrecisionV, MantissaT> v) {
            return v.template unshifted<double>();
        }
    };

} // namespace daw::json