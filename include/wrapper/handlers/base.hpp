#pragma once
#include <string>
#include "hdl_ctx.hpp"
#include "json_utils.hpp"
#include "virt_wrap.hpp"

template <boost::beast::http::verb... Vs> struct VerbList {
    constexpr static std::array<boost::beast::http::verb, sizeof...(Vs)> values = {Vs...}; // note: cannot use CTAD because of the empty list case
};

struct HandlerMethods : public HandlerContext {

    virtual DependsOutcome create(const rapidjson::Value& obj) = 0;
    virtual DependsOutcome query(const rapidjson::Value& obj) = 0;
    virtual DependsOutcome alter(const rapidjson::Value& obj) = 0;
    virtual DependsOutcome vacuum(const rapidjson::Value& obj) = 0;

    using Verbs =
        VerbList<boost::beast::http::verb::post, boost::beast::http::verb::get, boost::beast::http::verb::patch, boost::beast::http::verb::delete_>;
    constexpr static std::array methods = {&HandlerMethods::create, &HandlerMethods::query, &HandlerMethods::alter, &HandlerMethods::vacuum};

    static_assert(Verbs::values.size() == methods.size());

    explicit constexpr HandlerMethods(HandlerContext& hc) noexcept : HandlerContext(hc) {}

    constexpr static long verb_to_idx(boost::beast::http::verb v) noexcept {
        const auto it = cexpr::find(Verbs::values.begin(), Verbs::values.end(), v);
        return it != Verbs::values.end() ? std::distance(Verbs::values.begin(), it) : -1;
    }
};