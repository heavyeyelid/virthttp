//
// Created by hugo on 09.06.19.
//

#pragma once

#include <boost/beast/http/message.hpp>
#include <rapidjson/document.h>
#include "virt_wrap.hpp"

#include "handlers/domain.hpp"
#include "actions_table.hpp"
#include "fwd.hpp"
#include "json_utils.hpp"
#include "logger.hpp"
#include "urlparser.hpp"

namespace beast = boost::beast;
namespace http = beast::http;

constexpr std::string_view bsv2stdsv(boost::string_view bsv) noexcept { return {bsv.data(), bsv.length()}; }
constexpr boost::string_view stdsv2bsv(std::string_view sv) noexcept { return {sv.data(), sv.length()}; }

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
            else
                key_str = target.getPath().substr(key_start + 9);
        } else if (path.starts_with(stdsv2bsv(sv_by_name))) {
            search_key = SearchKey::by_name;
            const auto post_skey = path.substr(sv_by_name.length());
            if (post_skey.empty() || post_skey.substr(1).empty())
                return error(103), false;
            else
                key_str = bsv2stdsv(post_skey.substr(1));
        } else if (!path.empty() && !path.substr(1).empty()) {
            key_str = bsv2stdsv(path.substr(1));
            search_key = SearchKey::by_name;
        }
        return true;
    };

    auto domains = [&](virt::Connection&& conn) {
        HandlerContext hdl_ctx{conn, json_res, key_str};
        virt::Domain dom{};
        DomainHandlers hdls{hdl_ctx, dom};

        if (!getSearchKey("domains"))
            return;

        if (search_key != SearchKey::none) {

            if (search_key == SearchKey::by_name) {
                logger.debug("Getting domain by name");
                dom = conn.domainLookupByName(key_str.c_str());
                if (!dom) {
                    logger.error("Cannot find domain with name: ", key_str);
                    return error(100);
                }
            } else if (search_key == SearchKey::by_uuid) {
                logger.debug("Getting domain by uuid");
                dom = conn.domainLookupByUUIDString(key_str.c_str());
                if (!dom) {
                    logger.error("Cannot find domain with UUID: ", key_str);
                    return error(101);
                }
            }
            rapidjson::Document json_req{};
            json_req.Parse(req.body().data());
            switch (req.method()) {
            case http::verb::patch: {
                if (json_req.IsObject())
                    return (void)hdls.modification(json_req);
                if (json_req.IsArray())
                    return (void)handle_depends(json_req, json_res, [&](const auto& action) { return hdls.modification(action); });
                return error(3);
            } break;
            case http::verb::get: {
                return (void)hdls.query(json_req);
            }
            default: {
            }
            }
        } else {
            switch (req.method()) {
            case http::verb::get: {
                /* Filters extraction */
                auto flags = virt::Connection::List::Domains::Flags::DEFAULT;
                if (auto activity = target.getBool("active"); activity)
                    flags |= *activity ? virt::Connection::List::Domains::Flags::ACTIVE : virt::Connection::List::Domains::Flags::INACTIVE;
                if (auto persistence = target.getBool("persistent"); persistence)
                    flags |= *persistence ? virt::Connection::List::Domains::Flags::PERSISTENT : virt::Connection::List::Domains::Flags::TRANSIENT;
                if (auto savemgmt = target.getBool("managed_save"); savemgmt)
                    flags |= *savemgmt ? virt::Connection::List::Domains::Flags::MANAGEDSAVE : virt::Connection::List::Domains::Flags::NO_MANAGEDSAVE;
                if (auto autostart = target.getBool("autostart"); autostart)
                    flags |= *autostart ? virt::Connection::List::Domains::Flags::AUTOSTART : virt::Connection::List::Domains::Flags::NO_AUTOSTART;
                if (auto snapshot = target.getBool("has_snapshot"); snapshot)
                    flags |= *snapshot ? virt::Connection::List::Domains::Flags::HAS_SNAPSHOT : virt::Connection::List::Domains::Flags::NO_SNAPSHOT;

                auto [tag_name, tag_uuid, tag_status] = std::make_tuple(target["name"], target["uuid"], target["status"]);
                if (!tag_status.empty()) {
                    virt::Domain::State status;
                    if (const auto v = virt::Domain::States[tag_status]; v)
                        status = *v;
                    else
                        return error(301);
                    switch (status) {
                    case virt::Domain::State::RUNNING:
                        flags |= virt::Connection::List::Domains::Flags::RUNNING;
                        tag_status = {};
                        break;
                    case virt::Domain::State::PAUSED:
                        flags |= virt::Connection::List::Domains::Flags::PAUSED;
                        tag_status = {};
                        break;
                    case virt::Domain::State::SHUTOFF:
                        flags |= virt::Connection::List::Domains::Flags::SHUTOFF;
                        tag_status = {};
                        break;
                    default:
                        flags |= virt::Connection::List::Domains::Flags::OTHER;
                    }
                }
                for (const auto& dom : conn.listAllDomains(flags)) {
                    rapidjson::Value res_val;
                    res_val.SetObject();
                    const auto info = dom.getInfo();
                    res_val.AddMember("name", rapidjson::Value(dom.getName(), json_res.GetAllocator()), json_res.GetAllocator());
                    res_val.AddMember("uuid", dom.extractUUIDString(), json_res.GetAllocator());
                    res_val.AddMember("id", dom.getID(), json_res.GetAllocator());
                    res_val.AddMember("status", rapidjson::StringRef(virt::Domain::States[info.state]), json_res.GetAllocator());
                    /* Manual filters application */
                    if (!tag_name.empty() && tag_name != dom.getName())
                        continue;
                    if (!tag_uuid.empty() && tag_uuid != dom.extractUUIDString())
                        break;
                    if (!tag_status.empty() && (tag_status != virt::Domain::States[info.state] && tag_status != std::to_string(info.state)))
                        continue;
                    json_res.result(res_val);
                }
            } break;
            case http::verb::post: {
                rapidjson::Document json_req{};
                json_req.Parse(req.body().data());
                if (json_req.IsString())
                    return (void)hdls.creation(json_req, true);
                if (json_req.IsObject())
                    return (void)hdls.creation(json_req, false, true);
                if (json_req.IsArray())
                    return (void)handle_depends(json_req, json_res, [&](const rapidjson::Value& obj) { return hdls.creation(obj); });
                return error(0);
            } break;
            default: {
            }
            }
        }
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