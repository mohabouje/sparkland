#pragma once

#include "spl/concepts/types.hpp"
#include "spl/concepts/optional.hpp"
#include "spl/types/decimal.hpp"
#include "spl/reflect/object.hpp"

#include <daw/json/daw_json_link.h>

#include <unordered_map>
#include <map>

namespace spl::codec::json::contract {

    template <typename...>
    struct unnamed_mapper;

    template <spl::concepts::scoped_enum EnumT>
    struct unnamed_mapper<EnumT> {
        using type = daw::json::json_class_no_name<EnumT>;
    };

    template <spl::concepts::scoped_enum EnumT>
    struct unnamed_mapper<std::optional<EnumT>> {
        using type = daw::json::json_class_null_no_name<std::optional<EnumT>>;
    };

    template <spl::concepts::strictly_numeric NumericT>
    struct unnamed_mapper<NumericT> {
        using type = daw::json::json_number_no_name<NumericT>;
    };

    template <spl::concepts::boolean BooleanT>
    struct unnamed_mapper<BooleanT> {
        using type = daw::json::json_bool_no_name<BooleanT>;
    };

    template <spl::concepts::string StringT>
    struct unnamed_mapper<StringT> {
        using type = daw::json::json_string_no_name<StringT>;
    };

    template <std::ranges::range RangeT>
    requires(not spl::concepts::string<RangeT>)
    struct unnamed_mapper<RangeT> {
        using value_type = std::decay_t<typename std::ranges::range_value_t<RangeT>>;
        using type       = daw::json::json_array_no_name<typename unnamed_mapper<value_type>::type, RangeT>;
    };

    template <concepts::object ObjectT>
    struct unnamed_mapper<ObjectT> {
        using type = daw::json::json_class_no_name<ObjectT>;
    };

    template <spl::concepts::strictly_numeric NumericT>
    struct unnamed_mapper<std::optional<NumericT>> {
        using type = daw::json::json_number_null_no_name<std::optional<NumericT>>;
    };

    template <spl::concepts::boolean BooleanT>
    struct unnamed_mapper<std::optional<BooleanT>> {
        using type = daw::json::json_bool_null_no_name<std::optional<BooleanT>>;
    };

    template <spl::concepts::string StringT>
    struct unnamed_mapper<std::optional<StringT>> {
        using type = daw::json::json_string_null_no_name<std::optional<StringT>>;
    };

    template <std::ranges::range RangeT>
    requires(not spl::concepts::string<RangeT>)
    struct unnamed_mapper<std::optional<RangeT>> {
        using value_type = std::decay_t<typename std::ranges::range_value_t<RangeT>>;
        using type       = daw::json::json_array_null_no_name<typename unnamed_mapper<value_type>::type, RangeT>;
    };

    template <concepts::object ObjectT>
    struct unnamed_mapper<std::optional<ObjectT>> {
        using type = daw::json::json_class_null_no_name<std::optional<ObjectT>>;
    };

    template <std::int16_t PrecisionV, concepts::strictly_integral MantissaT>
    struct unnamed_mapper<types::decimal<PrecisionV, MantissaT>> {
        using type = daw::json::json_class_null_no_name<types::decimal<PrecisionV, MantissaT>>;
    };

    template <std::int16_t PrecisionV, concepts::strictly_integral MantissaT>
    struct unnamed_mapper<std::optional<types::decimal<PrecisionV, MantissaT>>> {
        using type = daw::json::json_class_null_no_name<std::optional<types::decimal<PrecisionV, MantissaT>>>;
    };

    template <>
    struct unnamed_mapper<std::string_view> {
        using type = daw::json::json_string_raw_no_name<std::string_view>;
    };

    template <>
    struct unnamed_mapper<std::optional<std::string_view>> {
        using type = daw::json::json_string_raw_null_no_name<std::optional<std::string_view>>;
    };

    template <typename RepT, typename AnotherT>
    struct unnamed_mapper<std::chrono::duration<RepT, AnotherT>> {
        using type = daw::json::json_class_no_name<std::chrono::duration<RepT, AnotherT>>;
    };

    template <typename RepT, typename AnotherT>
    struct unnamed_mapper<std::optional<std::chrono::duration<RepT, AnotherT>>> {
        using type = daw::json::json_class_null_no_name<std::optional<std::chrono::duration<RepT, AnotherT>>>;
    };

    template <typename... TypeT>
    struct unnamed_mapper<std::tuple<TypeT...>> {
        using type = daw::json::json_tuple_no_name<std::tuple<TypeT...>>;
    };

    template <typename... TypeT>
    struct unnamed_mapper<std::optional<std::tuple<TypeT...>>> {
        using type = daw::json::json_tuple_null_no_name<std::optional<std::tuple<TypeT...>>>;
    };

    template <typename... TypeT>
    struct unnamed_mapper<std::variant<TypeT...>> {
        using type =
            daw::json::json_variant_no_name<std::variant<TypeT...>, daw::json::json_variant_type_list<TypeT...>>;
    };

    template <typename... TypeT>
    struct unnamed_mapper<std::optional<std::variant<TypeT...>>> {
        using type = daw::json::json_variant_null_no_name<std::optional<std::variant<TypeT...>>>;
    };

    template <typename KeyT, typename ValueT>
    struct unnamed_mapper<std::unordered_map<KeyT, ValueT>> {
        using type = daw::json::json_key_value_no_name<std::unordered_map<KeyT, ValueT>, ValueT, KeyT>;
    };

    template <typename KeyT, typename ValueT>
    struct unnamed_mapper<std::optional<std::unordered_map<KeyT, ValueT>>> {
        using type =
            daw::json::json_key_value_null_no_name<std::optional<std::unordered_map<KeyT, ValueT>>, ValueT, KeyT>;
    };

    template <typename KeyT, typename ValueT>
    struct unnamed_mapper<std::map<KeyT, ValueT>> {
        using type = daw::json::json_key_value_no_name<std::map<KeyT, ValueT>, ValueT, KeyT>;
    };

    template <typename KeyT, typename ValueT>
    struct unnamed_mapper<std::optional<std::map<KeyT, ValueT>>> {
        using type = daw::json::json_key_value_null_no_name<std::optional<std::map<KeyT, ValueT>>, ValueT, KeyT>;
    };

} // namespace spl::codec::json::contract
