//
// Created by hugo on 09.06.19.
//

#pragma once

#include <boost/beast/http/message.hpp>
#include <rapidjson/document.h>
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
    auto target = TargetParser{bsv2stdsv(req.target())};

    enum class SearchKey { by_name, by_uuid, none } search_key = SearchKey::none;

    JsonRes json_res{};
    std::string key_str{};

    auto error = [&](auto... args) { return json_res.error(args...); };
    auto getSearchKey = [&](const std::string& type) {
        const auto key_start = "/libvirt/"sv.length() + type.size();
        const auto path = stdsv2bsv(target.getPath().substr(key_start)); // C++2a: use std::string_view all the way
        const auto sv_by_uuid = "/by-uuid"sv;
        const auto sv_by_name = "/by-name"sv;
        if (path.starts_with(stdsv2bsv(sv_by_uuid))) {
            search_key = SearchKey::by_uuid;
            const auto post_skey = path.substr(sv_by_uuid.length());
            if (post_skey.empty() || post_skey.substr(1).empty())
                return error(102), false;
            key_str = target.getPath().substr(key_start + 9);
        } else if (path.starts_with(stdsv2bsv(sv_by_name))) {
            search_key = SearchKey::by_name;
            const auto post_skey = path.substr(sv_by_name.length());
            if (post_skey.empty() || post_skey.substr(1).empty())
                return error(103), false;
            key_str = bsv2stdsv(post_skey.substr(1));
        } else if (!path.empty() && !path.substr(1).empty()) {
            key_str = bsv2stdsv(path.substr(1));
            search_key = SearchKey::by_name;
        }
        return true;
    };

    constexpr Resolver domain_resolver{tp<virt::Domain, DomainUnawareHandlers>, "domains", std::array{"by-name"sv, "by-uuid"sv},
                                       std::array{+[](const HandlerContext& hc, std::string_view sv) {
                                                      return hc.conn.domainLookupByName({sv.data(), sv.length()});
                                                  },
                                                  +[](const HandlerContext& hc, std::string_view sv) {
                                                      return hc.conn.domainLookupByUUIDString({sv.data(), sv.length()});
                                                  }},
                                       [](HandlerContext& hc, auto flags) { return hc.conn.listAllDomains(flags); }};

    auto domains = [&](virt::Connection&& conn) {
        HandlerContext hdl_ctx{conn, json_res, key_str};
        virt::Domain dom{};
        DomainHandlers hdls{hdl_ctx, dom};

        const auto doms = domain_resolver(target, hdl_ctx);
        const auto idx = HandlerMethods::verb_to_idx(req.method());
        if (idx < 0)
            return error(3);
        const auto mth = HandlerMethods::methods[idx];
        rapidjson::Document json_req{};
        json_req.Parse(req.body().data());

        auto exec = domain_jdispatchers[idx](json_req, [&](const auto& jval) { return (hdls.*mth)(jval); });
        exec(hdls);
    };

    auto networks = [&](virt::Connection&& conn) {
        if (!getSearchKey("networks"))
            return;
        switch (req.method()) {
        case http::verb::get: {
            if (search_key != SearchKey::none) {
                virt::Network nw{};
                if (search_key == SearchKey::by_name) {
                    logger.debug("Network by name");
                    nw = conn.networkLookupByName(key_str.c_str());
                    if (!nw) {
                        logger.error("Cannot find network with a such name");
                        return error(501);
                    }
                } else if (search_key == SearchKey::by_uuid) {
                    logger.debug("Network by UUID");
                    nw = conn.networkLookupByUUIDString(key_str.c_str());
                    if (!nw) {
                        logger.error("Cannot find network with a such name");
                        return error(502);
                    }
                }

                rapidjson::Value nw_json{};
                nw_json.SetObject();

                rapidjson::Value jsAS{};
                TFE nwAS = nw.getAutostart();
                if (!nwAS.err()) {
                    if (nwAS)
                        jsAS.SetBool(true);
                    else
                        jsAS.SetBool(false);
                } else {
                    logger.error("Error occurred while getting network autostart");
                    return error(503);
                }

                TFE nwActive = nw.isActive();
                rapidjson::Value jsonActive;
                if (!nwActive.err()) {
                    if (nwActive)
                        jsonActive.SetBool(true);
                    else
                        jsonActive.SetBool(false);
                } else {
                    logger.error("Error occurred while getting network status");
                    return error(500);
                }

                nw_json.AddMember("name", nw.extractName(), json_res.GetAllocator());
                nw_json.AddMember("uuid", nw.extractUUIDString(), json_res.GetAllocator());
                nw_json.AddMember("active", jsonActive, json_res.GetAllocator());
                nw_json.AddMember("autostart", jsAS, json_res.GetAllocator());
                json_res.result(nw_json);

            } else {
                logger.debug("Listing all networks");
                for (const auto& nw : conn.extractAllNetworks()) {
                    TFE nwActive = nw.isActive();
                    rapidjson::Value jsonActive;
                    if (!nwActive.err()) {
                        if (nwActive)
                            jsonActive.SetBool(true);
                        else
                            jsonActive.SetBool(false);
                    } else {
                        logger.error("Error occurred while getting network status");
                        return error(500);
                    }
                    rapidjson::Value nw_json;
                    nw_json.SetObject();
                    nw_json.AddMember("name", rapidjson::Value(nw.getName(), json_res.GetAllocator()), json_res.GetAllocator());
                    nw_json.AddMember("active", jsonActive, json_res.GetAllocator());
                    nw_json.AddMember("uuid", nw.extractUUIDString(), json_res.GetAllocator());
                    json_res.result(nw_json);
                }
            }
        } break;
        default: {
        }
        }
    };

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