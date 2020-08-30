//
// Created by hugo on 09.06.19.
//

#pragma once

#include <utility>
#include <boost/beast/http/message.hpp>
#include <boost/json.hpp>
#include "handlers/domain.hpp"
#include "handlers/network.hpp"
#include "actions_table.hpp"
#include "dispatch.hpp"
#include "general_store.hpp"
#include "json_utils.hpp"
#include "logger.hpp"
#include "solver.hpp"
#include "urlparser.hpp"
#include "virt_wrap.hpp"

namespace beast = boost::beast;
namespace http = beast::http;

template <class Body, class Allocator>
boost::json::string handle_json(GeneralStore& gstore, const http::request<Body, http::basic_fields<Allocator>>& req, const TargetParser& target) {
    JsonRes json_res{};
    auto error = [&](auto... args) { return json_res.error(args...); };

    auto object = [&](virt::Connection&& conn, auto resolver, auto jdispatchers, auto t_hdls) -> void {
        using Object = typename decltype(resolver)::O;
        using Handlers = typename decltype(t_hdls)::Type;
        HandlerContext hdl_ctx{conn, json_res, target};
        Object obj{};
        Handlers hdls{hdl_ctx, obj};

        auto skip_resolve = req.method() == http::verb::post;
        auto objs = !skip_resolve ? resolver(hdl_ctx) : std::vector<Object>{};
        const auto idx = HandlerMethods::verb_to_idx(req.method());
        if (idx < 0)
            return error(3);
        const auto mth = HandlerMethods::methods[idx];
        const auto& body = req.body();
        boost::json::value json_req{};
        if (!body.empty()) {
            std::error_code ec;
            json_req = boost::json::parse(req.body(), ec);
            if (ec)
                return error(8);
        }

        auto exec = jdispatchers[idx](json_req, [&](const auto& jval) { return (hdls.*mth)(jval); });
        if (skip_resolve)
            exec(hdls);
        else
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

    constexpr static std::array keys = {"domains"sv, "networks"sv};
    std::tuple fcns = {std::bind(object, std::placeholders::_1, domain_resolver, domain_jdispatchers, t_<DomainHandlers>),
                       std::bind(object, std::placeholders::_1, network_resolver, network_jdispatchers, t_<NetworkHandlers>)};

    [&] {
        auto& config = gstore.config();
        if (config.isHTTPAuthRequired() && req["X-Auth-Key"] != config.http_auth_key)
            return error(1);
        auto path_parts = target.getPathParts();
        if (path_parts.empty())
            return error(4); // Empty request (/)
        if (path_parts.front() != "libvirt")
            return error(5); // Path does not start by /libvirt
        if (path_parts.size() <= 1)
            return error(6); // Path is only /libvirt

        logger.debug("Opening connection to ", config.getConnURI());
        virt::Connection conn{config.connURI.c_str()};

        if (!conn) {
            logger.error("Failed to open connection to ", config.getConnURI());
            return error(10);
        }

        const auto it = std::find(keys.begin(), keys.end(), path_parts[1]);
        if (it == keys.end())
            return error(7);

        int i = std::distance(keys.begin(), it);
        return visit(fcns, [&](const auto& e) {
            if (i-- == 0)
                e(std::move(conn));
        });
    }();

    return boost::json::to_string(json_res);
}