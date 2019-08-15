#pragma once
#include <array>
#include <boost/beast/http/verb.hpp>
#include <gsl/gsl>
#include <rapidjson/document.h>
#include "handlers/hdl_ctx.hpp"
#include "cexpr_algs.hpp"
#include "depends.hpp"

template <rapidjson::Type... Vs> struct JTypeList {
    constexpr static std::array<rapidjson::Type, sizeof...(Vs)> values = {Vs...}; // note: cannot use CTAD because of the empty list case
};

template <class S, class R = JTypeList<>> class JDispatchVals;
class JDispatch;

using JAll = JTypeList<static_cast<rapidjson::Type>(-1)>;

template <rapidjson::Type... Ss, rapidjson::Type... Rs> class JDispatchVals<JTypeList<Ss...>, JTypeList<Rs...>> {
    friend JDispatch;
    constexpr static auto singles = JTypeList<Ss...>{}.values;
    constexpr static auto ranges = JTypeList<Rs...>{}.values;

    constexpr static bool has_one_in_both() noexcept {
        for (auto s : singles) {
            for (auto r : ranges) {
                if (r == s)
                    return true;
            }
        }
        return false;
    }
    static_assert(!has_one_in_both());
};

class JDispatch {
    gsl::span<const rapidjson::Type> singles; // C++2a use std::span
    gsl::span<const rapidjson::Type> ranges;  // C++2a use std::span

  public:
    template <class JDV>
    explicit constexpr JDispatch(const JDV& jdv) noexcept
        : singles(jdv.singles.data(), jdv.singles.size()), ranges(jdv.ranges.data(), jdv.ranges.size()) {}
    template <class Hdl> auto operator()(const rapidjson::Value& jval, Hdl&& hdl) const {
        return [&, this, hdl](HandlerContext& hc) {
            const auto jtype = jval.GetType();
            if (!singles.empty() && static_cast<int>(singles[0]) == -1) {
                return (void)hdl(jval);
            } else {
                if (cexpr::find(singles.begin(), singles.end(), jtype) != singles.end())
                    return (void)hdl(jval);
            }
            if (cexpr::find(ranges.begin(), ranges.end(), jtype) != ranges.end())
                return handle_depends(jval, hc.json_res, hdl);
            return hc.json_res.error(3);
        };
    }
};

template <class... JDVs, std::size_t... I>
constexpr auto gen_jdispatchers_impl(const std::tuple<JDVs...>& tup, std::index_sequence<I...>) noexcept -> std::array<JDispatch, sizeof...(JDVs)> {
    constexpr auto mk_jdisp = [](const auto& jdv) { return JDispatch{jdv}; };
    return std::array{mk_jdisp(std::get<I>(tup))...};
}

template <class... JDVs> constexpr auto gen_jdispatchers(const std::tuple<JDVs...>& tup) noexcept -> std::array<JDispatch, sizeof...(JDVs)> {
    return gen_jdispatchers_impl(tup, std::index_sequence_for<JDVs...>{});
}