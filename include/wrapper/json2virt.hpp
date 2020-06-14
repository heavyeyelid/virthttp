#pragma once
#include <boost/json.hpp>
#include "../utils.hpp"
#include "virt_wrap/TypesParam.hpp"
#include "json_utils.hpp"

std::optional<virt::TypedParams> json_to_typed_parameters(const boost::json::value& val);

enum class JTag {
    None,
    // Object,
    Array,
    Bool,
    Int64,
    Uint64,
    Double,
    String,
    Enum,
};

template <JTag tag, class = void> struct JTagRepr { using type = void; };

template <> struct JTagRepr<JTag::Bool, void> { using type = bool; };
template <> struct JTagRepr<JTag::Int64, void> { using type = long long; };
template <> struct JTagRepr<JTag::Uint64, void> { using type = unsigned long long; };
template <> struct JTagRepr<JTag::Double, void> { using type = double; };
template <> struct JTagRepr<JTag::String, void> { using type = std::string; };
template <class F> struct JTagRepr<JTag::Enum, F> { using type = F; };

template <class> struct JTagReverse;

template <JTag tag_, class Extra_> struct JTagReverse<JTagRepr<tag_, Extra_>> {
    using Extra = Extra_;
    constexpr static JTag tag = tag_;
};

template <JTag tag, class Extra = void> using JTagRepr_t = typename JTagRepr<tag, Extra>::type;

template <JTag type, JTag nested_type = JTag::None, class Extra = void>
auto extract_param_val(const boost::json::value& el_json, JsonRes& json_res) noexcept {
    static_assert(type != JTag::None, "Extracted type must exist");
    using SubRepr = JTagRepr_t<nested_type, Extra>;
    using RawRepr = JTagRepr<type, Extra>;
    using Repr = std::conditional_t<type != JTag::Array, JTagRepr_t<type, Extra>, std::vector<SubRepr>>;
    using Ret = std::optional<Repr>;
    auto error = [&](auto... args) { return json_res.error(args...), Ret{std::nullopt}; };

    Ret ret{};
    auto& ret_val = ret.emplace();

    if constexpr (type == JTag::Array) {
        static_assert(nested_type != JTag::None, "Sub-extracted type must exist");

        if (!el_json.is_array())
            return error(0);
        const auto el_jsonarr = el_json.get_array();
        ret_val.reserve(el_jsonarr.size());
        for (const auto& sub_el : el_jsonarr) {
            auto res = extract_param_val<nested_type, JTag::None>(sub_el, json_res);
            if (!res)
                return Ret{std::nullopt};
            ret_val.push_back(std::move(*res));
        }
    } else {
        static_assert(nested_type == JTag::None, "Sub-extracted type shall not exist");
        if constexpr (type == JTag::Bool) {
            if (!el_json.is_bool())
                return error(0);
            ret_val = el_json.get_bool();
        } else if constexpr (type == JTag::Int64) {
            if (!el_json.is_int64())
                return error(0);
            ret_val = el_json.get_int64();
        } else if constexpr (type == JTag::Uint64) {
            if (!el_json.is_uint64())
                return error(0);
            ret_val = el_json.get_uint64();
        } else if constexpr (type == JTag::Double) {
            if (!el_json.is_double())
                return error(0);
            ret_val = el_json.get_double();
        } else if constexpr (type == JTag::String) {
            if (!el_json.is_string())
                return error(0);
            ret_val = el_json.get_string();
        } else if constexpr (type == JTag::Enum) {
            if (!el_json.is_string())
                return error(0);
            const auto enum_opt = Repr::from_string(el_json.get_string());
            if (!enum_opt)
                return Ret{std::nullopt};
            ret_val = *enum_opt;
        }
    }
    return ret;
}

template <JTag type, JTag nested_type = JTag::None, class Extra = void>
auto extract_param(const boost::json::object& val, gsl::czstring<> name, JsonRes& json_res, bool consider_optional = false) noexcept
    -> decltype(extract_param_val<type, nested_type, Extra>(std::declval<decltype(val)>(), json_res)) {
    using Ret = decltype(extract_param_val<type, nested_type, Extra>(std::declval<decltype(val)>(), json_res));
    const auto el_it = val.find(name);
    if (el_it == val.end()) {
        if (consider_optional) {
            Ret ret{};
            ret.emplace(); // Requires the type to have a default ctor
            return ret;
        }
        return std::nullopt;
    }
    return extract_param_val<type, nested_type, Extra>(el_it->value(), json_res);
}

template <JTag tag, JTag nested_type_ = JTag::None, class Extra = void> struct WArg {
    using JTagRepr = ::JTagRepr<tag, Extra>;
    using Ret =
        RemoveOptional_t<decltype(extract_param_val<tag, nested_type_, Extra>(std::declval<const boost::json::value&>(), std::declval<JsonRes&>()))>;
    constexpr static auto nested_type = nested_type_;

    gsl::czstring<> name;
    bool optional = false;
};

template <size_t... I, class... Args>
auto wrap_fcn_args_impl(const boost::json::value& val, JsonRes& json_res, std::index_sequence<I...> is, Args... args) noexcept {
    using Tup = std::tuple<typename Args::Ret...>;
    std::optional<Tup> ret{};
    auto& vals = ret.emplace();

    if constexpr (sizeof...(args) > 1) {
        if (!val.is_object())
            return json_res.error(0), decltype(ret){std::nullopt};
        auto proc_parm = [&](auto warg, auto ic) {
            using JTagRev = JTagReverse<typename decltype(warg)::JTagRepr>;
            using Extra = typename JTagRev::Extra;
            constexpr JTag tag = JTagRev::tag;
            constexpr JTag nested_tag = decltype(warg)::nested_type;
            constexpr auto idx = decltype(ic)::value;
            const auto res = extract_param<tag, nested_tag, Extra>(val.get_object(), warg.name, json_res);
            if (res)
                std::get<idx>(vals) = std::move(*res);
            return static_cast<bool>(res);
        };
        return (proc_parm(args, std::integral_constant<size_t, I>{}) && ...) ? ret : std::nullopt; // Done this way to use lazy evaluation
    } else if constexpr (sizeof...(args) == 1) {
        auto proc_parm = [&](auto warg) {
            using JTagRev = JTagReverse<typename decltype(warg)::JTagRepr>;
            using Extra = typename JTagRev::Extra;
            constexpr JTag tag = JTagRev::tag;
            constexpr JTag nested_tag = decltype(warg)::nested_type;
            const auto res = extract_param_val<tag, nested_tag, Extra>(val, json_res);
            if (res)
                std::get<0>(vals) = std::move(*res);
            return static_cast<bool>(res);
        };
        return (proc_parm(args) && ...) ? ret : std::nullopt;
    } else {
        return std::optional{std::tuple<>{}};
    }
}

template <class... Args> auto wrap_fcn_args(const boost::json::value& val, JsonRes& json_res, Args... args) noexcept {
    return wrap_fcn_args_impl(val, json_res, std::index_sequence_for<Args...>{}, args...);
}

template <class Fcn, class... Args> auto wrap_fcn(const boost::json::value& val, JsonRes& json_res, Fcn fcn, Args... args) noexcept {
    const auto tup = wrap_fcn_args(val, json_res, args...);
    return tup ? std::optional{std::apply(fcn, *tup)} : std::nullopt;
}