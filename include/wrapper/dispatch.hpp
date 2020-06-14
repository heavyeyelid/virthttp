#pragma once
#include <array>
#include <boost/beast/http/verb.hpp>
#include <boost/json.hpp>
#include <gsl/gsl>
#include "handlers/hdl_ctx.hpp"
#include "cexpr_algs.hpp"
#include "depends.hpp"

/**
 * \internal
 * rapidjson::Type variadic template to array conversion class
 *
 * \tparam Vs the rapidjson value types
 **/
template <boost::json::kind... Vs> struct JTypeList {
    constexpr static std::array<boost::json::kind, sizeof...(Vs)> values = {Vs...}; // note: cannot use CTAD because of the empty list case
};

template <class S, class R = JTypeList<>> class JDispatchVals;
class JDispatch;

/**
 * A special JTypeList reprensenting the full set of JSON value types
 **/
using JAll = JTypeList<static_cast<boost::json::kind>(-1)>;

/**
 * \internal
 * JSON value type dispatching values
 *
 * \tparam Ss the value types allowed to be passed to the handler
 * \tparam Rs the container types allowed to be passed to the handler
 **/
template <boost::json::kind... Ss, boost::json::kind... Rs> class JDispatchVals<JTypeList<Ss...>, JTypeList<Rs...>> {
    friend JDispatch;
    constexpr static auto singles = JTypeList<Ss...>{}.values; ///< Value types allowed to be passed to the handler
    constexpr static auto ranges = JTypeList<Rs...>{}.values;  ///< Container types allowed to be passed to the handler

    /**
     * \internal
     * Checks whether #singles and #ranges share at least one JSON type in common
     *
     * \return `true` if the intersection of #singles and #ranges is not an empty set, `false` otherwise
     **/
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

/**
 * \internal
 * De-templatized reference to a JDispatchVals so it can be stored in an array
 **/
class JDispatch {
    gsl::span<const boost::json::kind> singles; // C++2a use std::span
    gsl::span<const boost::json::kind> ranges;  // C++2a use std::span

  public:
    /**
     * \internal
     * Constructs a JDispatch from a JDispatcherVals
     *
     * \tparam JDV (deduced)
     * \param[in] jdv JDispatcherVals object to use as a source
     **/
    template <class JDV> explicit constexpr JDispatch(const JDV& jdv) noexcept : singles(jdv.singles), ranges(jdv.ranges) {}

    /**
     * \internal
     * Dispatching-closure factory
     *
     * \tparam Hdl (deduced)
     * \param[in] jval the JSON input value
     * \param[in] hdl the callable which will process jval if the JSON value type is allowed
     * \return a closure of signature void(HandlerContext&)
     **/
    template <class Hdl> auto operator()(const boost::json::value& jval, Hdl&& hdl) const {
        return [&, this, hdl = std::forward<Hdl>(hdl)](HandlerContext& hc) {
            const auto jkind = jval.kind();
            if (!singles.empty() && static_cast<int>(singles[0]) == -1) {
                return (void)hdl(jval);
            } else {
                if (cexpr::find(singles.begin(), singles.end(), jkind) != singles.end())
                    return (void)hdl(jval);
            }
            if (cexpr::find(ranges.begin(), ranges.end(), jkind) != ranges.end())
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

/**
 * \internal
 * Generates an array of JDispatch from a tuple of various JDispatcherVals
 *
 * \tparam JDV (deduced)
 * \param[in] tup the input tuple
 * \return the output array
 **/
template <class... JDVs> constexpr auto gen_jdispatchers(const std::tuple<JDVs...>& tup) noexcept -> std::array<JDispatch, sizeof...(JDVs)> {
    return gen_jdispatchers_impl(tup, std::index_sequence_for<JDVs...>{});
}