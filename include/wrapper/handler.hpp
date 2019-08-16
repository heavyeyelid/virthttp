//
// Created by hugo on 09.06.19.
//

#pragma once

#include <utility>
#include <boost/beast/http/message.hpp>
#include <rapidjson/document.h>
#include "handlers/domain.hpp"
#include "wrapper/handlers/network.hpp"
#include "actions_table.hpp"
#include "dispatch.hpp"
#include "fwd.hpp"
#include "json_utils.hpp"
#include "logger.hpp"
#include "solver.hpp"
#include "urlparser.hpp"
#include "virt_wrap.hpp"

namespace beast = boost::beast;
namespace http = beast::http;

template <class Body, class Allocator> rapidjson::StringBuffer handle_json(const http::request<Body, http::basic_fields<Allocator>>& req) {
    JsonRes json_res{};
    std::string key_str{};
    auto error = [&](auto... args) { return json_res.error(args...); };
    auto target = TargetParser{bsv2stdsv(req.target())};

    auto object = [&](virt::Connection&& conn, auto resolver, auto jdispatchers, auto t_hdls) -> void {
        using Object = typename decltype(resolver)::O;
        using Handlers = typename decltype(t_hdls)::Type;
        HandlerContext hdl_ctx{conn, json_res, target, key_str};
        Object obj{};
        Handlers hdls{hdl_ctx, obj};

        auto objs = resolver(target, hdl_ctx);
        const auto idx = HandlerMethods::verb_to_idx(req.method());
        if (idx < 0)
            return error(3);
        const auto mth = HandlerMethods::methods[idx];
        rapidjson::Document json_req{};
        json_req.Parse(req.body().data());

        auto exec = jdispatchers[idx](json_req, [&](const auto& jval) { return (hdls.*mth)(jval); });
        for (auto&& v : objs)
            obj = std::move(v), exec(hdls);
    };

    constexpr Resolver domain_resolver{tp<virt::Domain, DomainUnawareHandlers>, "domains", std::array{"by-name"sv, "by-uuid"sv},
                                       std::array{+[](const HandlerContext& hc, std::string_view sv) {
                                                      return hc.conn.domainLookupByName({sv.data(), sv.length()});
                                                  },
                                                  +[](const HandlerContext& hc, std::string_view sv) {
                                                      return hc.conn.domainLookupByUUIDString({sv.data(), sv.length()});
                                                  }},
                                       [](HandlerContext& hc, auto flags) { return hc.conn.listAllDomains(flags); }};

    constexpr Resolver network_resolver{tp<virt::Network, NetworkUnawareHandlers>, "networks", std::array{"by-name"sv, "by-uuid"sv},
                                        std::array{+[](const HandlerContext& hc, std::string_view sv) {
                                                       return hc.conn.networkLookupByName({sv.data(), sv.length()});
                                                   },
                                                   +[](const HandlerContext& hc, std::string_view sv) {
                                                       return hc.conn.networkLookupByUUIDString({sv.data(), sv.length()});
                                                   }},
                                        [](HandlerContext& hc, auto flags) { return hc.conn.extractAllNetworks(flags); }};

    constexpr static std::array keys = {"/libvirt/domains"sv, "/libvirt/networks"sv};
    std::tuple fcns = {std::bind(object, std::placeholders::_1, domain_resolver, domain_jdispatchers, t_<DomainHandlers>),
                       std::bind(object, std::placeholders::_1, network_resolver, network_jdispatchers, t_<NetworkHandlers>)};

    [&] {
        if (iniConfig.isHTTPAuthRequired() && req["X-Auth-Key"] != iniConfig.http_auth_key)
            return error(1);
        logger.debug("Opening connection to ", iniConfig.getConnURI());
        virt::Connection conn{iniConfig.connURI.c_str()};

        if (!conn) {
            logger.error("Failed to open connection to ", iniConfig.getConnURI());
            return error(10);
        }

        auto i = 0;

        for (const auto key : keys) {
            if (req.target().starts_with(stdsv2bsv(key)))
                return visit(fcns, [&](const auto& e) {
                    if (i-- == 0)
                        e(std::move(conn));
                });
            ++i;
        }
        return error(2);
    }();

    rapidjson::StringBuffer buffer;
    using UTF8_Writer = rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType, rapidjson::UTF8<>>;
    UTF8_Writer writer(buffer);
    json_res.Accept(writer);

    return buffer;
}