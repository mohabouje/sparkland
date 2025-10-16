#pragma once

#include "spl/concepts/types.hpp"
#include "spl/concepts/object.hpp"
#include "spl/types/decimal.hpp"
#include "spl/meta/tstring.hpp"

#include <daw/json/daw_json_link.h>

#include <chrono>
#include <unordered_map>
#include <map>

namespace spl::codec::json::contract {

    template <meta::tstring Name, typename...>
    struct named_mapper;

    template <meta::tstring Name, concepts::object ObjectT>
    struct named_mapper<Name, ObjectT> {
        using type = daw::json::json_class<Name.data, ObjectT>;
    };

    template <meta::tstring Name, spl::concepts::strictly_numeric NumericT>
    struct named_mapper<Name, NumericT> {
        using type = daw::json::json_number<Name.data, NumericT>;
    };

    template <meta::tstring Name, spl::concepts::boolean BooleanT>
    struct named_mapper<Name, BooleanT> {
        using type = daw::json::json_bool<Name.data, BooleanT>;
    };

    template <meta::tstring Name, spl::concepts::string StringT>
    struct named_mapper<Name, StringT> {
        using type = daw::json::json_string<Name.data, StringT>;
    };

    template <meta::tstring Name, typename RepT, typename AnotherT>
    struct named_mapper<Name, std::chrono::duration<RepT, AnotherT>> {
        using type = daw::json::json_class<Name.data, std::chrono::duration<RepT, AnotherT>>;
    };

    template <meta::tstring Name, std::ranges::range RangeT>
    requires(not spl::concepts::string<RangeT>)
    struct named_mapper<Name, RangeT> {
        using value_type = std::decay_t<typename std::ranges::range_value_t<RangeT>>;
        using type       = daw::json::json_array<Name.data, value_type, RangeT>;
    };

    template <meta::tstring Name, concepts::object ObjectT>
    struct named_mapper<Name, std::optional<ObjectT>> {
        using type = daw::json::json_class_null<Name.data, std::optional<ObjectT>>;
    };

    template <meta::tstring Name, concepts::scoped_enum EnumT>
    struct named_mapper<Name, EnumT> {
        using type = daw::json::json_class<Name.data, EnumT>;
    };

    template <meta::tstring Name, concepts::scoped_enum EnumT>
    struct named_mapper<Name, std::optional<EnumT>> {
        using type = daw::json::json_class_null<Name.data, std::optional<EnumT>>;
    };

    template <meta::tstring Name, spl::concepts::strictly_numeric NumericT>
    struct named_mapper<Name, std::optional<NumericT>> {
        using type = daw::json::json_number_null<Name.data, std::optional<NumericT>>;
    };

    template <meta::tstring Name, spl::concepts::boolean BooleanT>
    struct named_mapper<Name, std::optional<BooleanT>> {
        using type = daw::json::json_bool_null<Name.data, std::optional<BooleanT>>;
    };

    template <meta::tstring Name, spl::concepts::string StringT>
    struct named_mapper<Name, std::optional<StringT>> {
        using type = daw::json::json_string_null<Name.data, std::optional<StringT>>;
    };

    template <meta::tstring Name, typename RepT, typename AnotherT>
    struct named_mapper<Name, std::optional<std::chrono::duration<RepT, AnotherT>>> {
        using type = daw::json::json_class_null<Name.data, std::optional<std::chrono::duration<RepT, AnotherT>>>;
    };

    template <meta::tstring Name, std::ranges::range RangeT>
    requires(not spl::concepts::string<RangeT>)
    struct named_mapper<Name, std::optional<RangeT>> {
        using value_type = std::decay_t<typename std::ranges::range_value_t<RangeT>>;
        using type       = daw::json::json_array_null<Name.data, RangeT, value_type>;
    };

    template <meta::tstring Name, std::int16_t PrecisionV, concepts::strictly_integral MantissaT>
    struct named_mapper<Name, types::decimal<PrecisionV, MantissaT>> {
        using type = daw::json::json_class<Name.data, types::decimal<PrecisionV, MantissaT>>;
    };

    template <meta::tstring Name, std::int16_t PrecisionV, concepts::strictly_integral MantissaT>
    struct named_mapper<Name, std::optional<types::decimal<PrecisionV, MantissaT>>> {
        using type = daw::json::json_class_null<Name.data, std::optional<types::decimal<PrecisionV, MantissaT>>>;
    };

    template <meta::tstring Name>
    struct named_mapper<Name, std::string_view> {
        using type = daw::json::json_string_raw<Name.data, std::string_view>;
    };

    template <meta::tstring Name>
    struct named_mapper<Name, std::optional<std::string_view>> {
        using type = daw::json::json_string_raw_null<Name.data, std::optional<std::string_view>>;
    };

    template <meta::tstring Name, typename... TypeT>
    struct named_mapper<Name, std::tuple<TypeT...>> {
        using type = daw::json::json_tuple<Name.data, std::tuple<TypeT...>>;
    };

    template <meta::tstring Name, typename... TypeT>
    struct named_mapper<Name, std::optional<std::tuple<TypeT...>>> {
        using type = daw::json::json_tuple_null<Name.data, std::optional<std::tuple<TypeT...>>>;
    };

    template <meta::tstring Name, typename... TypeT>
    struct named_mapper<Name, std::variant<TypeT...>> {
        using type = daw::json::json_variant<Name.data, std::variant<TypeT...>>;
    };

    template <meta::tstring Name, typename... TypeT>
    struct named_mapper<Name, std::optional<std::variant<TypeT...>>> {
        using type = daw::json::json_variant_null<Name.data, std::optional<std::variant<TypeT...>>>;
    };

    template <meta::tstring Name, typename KeyT, typename ValueT>
    struct named_mapper<Name, std::unordered_map<KeyT, ValueT>> {
        using type = daw::json::json_key_value<Name.data, std::unordered_map<KeyT, ValueT>, ValueT, KeyT>;
    };

    template <meta::tstring Name, typename KeyT, typename ValueT>
    struct named_mapper<Name, std::optional<std::unordered_map<KeyT, ValueT>>> {
        using type =
            daw::json::json_key_value_null<Name.data, std::optional<std::unordered_map<KeyT, ValueT>>, ValueT, KeyT>;
    };

    template <meta::tstring Name, typename KeyT, typename ValueT>
    struct named_mapper<Name, std::map<KeyT, ValueT>> {
        using type = daw::json::json_key_value<Name.data, std::map<KeyT, ValueT>, ValueT, KeyT>;
    };

    template <meta::tstring Name, typename KeyT, typename ValueT>
    struct named_mapper<Name, std::optional<std::map<KeyT, ValueT>>> {
        using type = daw::json::json_key_value_null<Name.data, std::optional<std::map<KeyT, ValueT>>, ValueT, KeyT>;
    };

} // namespace spl::codec::json::contract
