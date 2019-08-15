//
// Created by hugo on 09.06.19.
//

#pragma once

#include <boost/beast/http/message.hpp>
#include <rapidjson/document.h>
#include "wrapper/handlers/network.hpp"
#include "virt_wrap.hpp"

#include "handlers/domain.hpp"
#include "actions_table.hpp"
#include "dispatch.hpp"
#include "fwd.hpp"
#include "json_utils.hpp"
#include "logger.hpp"
#include "solver.hpp"
#include "urlparser.hpp"

namespace beast = boost::beast;
namespace http = beast::http;

template <class Body, class Allocator> rapidjson::StringBuffer handle_json(const http::request<Body, http::basic_fields<Allocator>>& req) {
    JsonRes json_res{};
    std::string key_str{};
    auto error = [&](auto... args) { return json_res.error(args...); };
    auto target = TargetParser{bsv2stdsv(req.target())};

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

    auto object = [&](virt::Connection&& conn, auto resolver, auto jdispatchers, auto t_hdls) {
        using Object = typename decltype(resolver)::O;
        using Handlers = typename decltype(t_hdls)::Type;
        HandlerContext hdl_ctx{conn, json_res, key_str};
        Object dom{};
        Handlers hdls{hdl_ctx, dom};

        const auto doms = resolver(target, hdl_ctx);
        const auto idx = HandlerMethods::verb_to_idx(req.method());
        if (idx < 0)
            return error(3);
        const auto mth = HandlerMethods::methods[idx];
        rapidjson::Document json_req{};
        json_req.Parse(req.body().data());

        auto exec = jdispatchers[idx](json_req, [&](const auto& jval) { return (hdls.*mth)(jval); });
        exec(hdls);
    };

    auto domains = std::bind(object, std::placeholders::_1, domain_resolver, domain_jdispatchers, t_<DomainHandlers>);
    auto networks = std::bind(object, std::placeholders::_1, network_resolver, network_jdispatchers, t_<NetworkHandlers>);

    [&] {
        if (iniConfig.isHTTPAuthRequired() && req["X-Auth-Key"] != iniConfig.http_auth_key)
            return error(1);
        logger.debug("Opening connection to ", iniConfig.getConnURI());
        virt::Connection conn{iniConfig.connURI.c_str()};
        if (!conn) {
            logger.error("Failed to open connection to ", iniConfig.getConnURI());
            return error(10);
        }
        if (req.target().starts_with("/libvirt/domains"))
            return domains(std::move(conn));
        else if (req.target().starts_with("/libvirt/networks"))
            return networks(std::move(conn));
        else
            return error(2);
    }();

    rapidjson::StringBuffer buffer;
    using UTF8_Writer = rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType, rapidjson::UTF8<>>;
    UTF8_Writer writer(buffer);
    json_res.Accept(writer);

    return buffer;
}