//
// Created by hugo on 09.06.19.
//

#pragma once

#include <boost/beast/http/message.hpp>
#include <rapidjson/document.h>
#include "virt_wrap.hpp"

#include "actions_table.hpp"
#include "json_utils.hpp"
#include "fwd.hpp"
#include "logger.hpp"
#include "urlparser.hpp"

namespace beast = boost::beast;
namespace http = beast::http;

constexpr std::string_view bsv2stdsv(boost::string_view bsv) noexcept { return {bsv.data(), bsv.length()}; }

template <class Body, class Allocator> rapidjson::StringBuffer handle_json(http::request<Body, http::basic_fields<Allocator>>&& req) {
    auto target = TargetParser{bsv2stdsv(req.target())};

    enum class SearchKey { by_name, by_uuid, none } search_key = SearchKey::none;

    JsonRes json_res{};

    std::string key_str{};

    auto error = [&](auto... args) { return json_res.error(args...); };

    auto getSearchKey = [&](const std::string& type) {
        auto key_start = std::string{"/libvirt/" + type}.length();
        if (target.getPath().substr(key_start, 8).compare("/by-uuid") == 0) {
            search_key = SearchKey::by_uuid;
            if (target.getPath().substr(key_start + 8).empty() || target.getPath().substr(key_start + 9).empty())
                return error(102, "No UUID specified"), false;
            else
                key_str = target.getPath().substr(key_start + 9);
        } else if (target.getPath().substr(key_start, 8).compare("/by-name") == 0) {
            search_key = SearchKey::by_name;
            if (target.getPath().substr(key_start + 8).empty() || target.getPath().substr(key_start + 9).empty())
                return error(103, "No name specified"), false;
            else
                key_str = target.getPath().substr(key_start + 9);
        } else if (!target.getPath().substr(key_start).empty() && !target.getPath().substr(key_start + 1).empty()) {
            key_str = target.getPath().substr(key_start + 1);
            search_key = SearchKey::by_name;
        }
        return true;
    };

    auto domains = [&](const virt::Connection& conn) {
        if (!getSearchKey("domains"))
            return;

        if (search_key != SearchKey::none) {
            virt::Domain dom{};
            if (search_key == SearchKey::by_name) {
                logger.debug("Getting domain by name");
                dom = conn.domainLookupByName(key_str.c_str());
                if (!dom) {
                    logger.error("Cannot find domain with name: ", key_str);
                    return error(100, "Cannot find domain with a such name");
                }
            } else if (search_key == SearchKey::by_uuid) {
                logger.debug("Getting domain by uuid");
                dom = conn.domainLookupByUUIDString(key_str.c_str());
                if (!dom) {
                    logger.error("Cannot find domain with UUID: ", key_str);
                    return error(101, "Cannot find domain with a such UUID");
                }
            }
            switch (req.method()) {
            case http::verb::patch: {
                rapidjson::Document json_req{};
                json_req.Parse(req.body().data());
                if (!json_req.IsArray())
                    return error(298, "PATCH data has to be an array of actions");

                std::vector<ActionOutcome> outcomes{};
                outcomes.reserve(json_req.Size());
                for (const auto& action : json_req.GetArray()) {
                    const auto curr_idx = outcomes.size();
                    const auto& action_obj = *action.MemberBegin();
                    const auto& [action_name, action_val] = action_obj;

                    if ([&]() noexcept {
                            const auto it = action.FindMember("depends");
                            if (it == action.MemberEnd())
                                return true;

                            const auto& json_deps = it->value;
                            auto success_pred = [&](const auto& dep) {
                                if (!dep.IsInt() || dep.GetInt() >= curr_idx || outcomes[dep.GetInt()] != ActionOutcome::SUCCESS)
                                    return outcomes.push_back(ActionOutcome::SKIPPED), false;
                                return true;
                            };
                            if (json_deps.IsArray()) {
                                const auto deps_arr = json_deps.GetArray();
                                return std::all_of(deps_arr.begin(), deps_arr.end(), success_pred);
                            }
                            if (json_deps.IsInt()) {
                                return success_pred(json_deps);
                            }
                            return error(0, "Syntax error"), outcomes.push_back(ActionOutcome::FAILURE), false;
                        }()) {
                        const auto hdl = domain_actions_table[std::string_view{action_name.GetString(), action_name.GetStringLength()}];
                        outcomes.push_back(hdl ? hdl(action_val, json_res, dom, key_str) : (error(123, "Unknown action"), ActionOutcome::FAILURE));
                    }
                }
            } break;
            case http::verb::get: {
                rapidjson::Value res_val;
                res_val.SetObject();
                const auto [state, max_mem, memory, nvirt_cpu, cpu_time] = dom.getInfo();
                const auto os_type = dom.getOSType();
                res_val.AddMember("name", rapidjson::Value(dom.getName(), json_res.GetAllocator()), json_res.GetAllocator());
                res_val.AddMember("uuid", dom.extractUUIDString(), json_res.GetAllocator());
                res_val.AddMember("status", rapidjson::StringRef(virt::Domain::States[state]), json_res.GetAllocator());
                res_val.AddMember("os", rapidjson::Value(os_type.get(), json_res.GetAllocator()), json_res.GetAllocator());
                res_val.AddMember("ram", memory, json_res.GetAllocator());
                res_val.AddMember("ram_max", max_mem, json_res.GetAllocator());
                res_val.AddMember("cpu", nvirt_cpu, json_res.GetAllocator());

                json_res.result(res_val);
            } break;
            default: {
            }
            }
        } else {
            if (req.method() == http::verb::get) {
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
                    if(std::all_of(tag_status.cbegin(), tag_status.cend(), [&](auto c) { return std::isdigit(c); }))
                        status = virt::Domain::State(std::stol(std::string{tag_status}));
                    else if(const auto v = virt::Domain::States[tag_status]; v)
                        status = *v;
                    else
                        return error(301, "Invalid flag");
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
                    res_val.AddMember("status", rapidjson::StringRef(virt::Domain::States[info.state]), json_res.GetAllocator());
                    if (!tag_name.empty() && tag_name != dom.getName())
                        continue;
                    if (!tag_uuid.empty() && tag_uuid != dom.extractUUIDString())
                        continue;
                    if (!tag_status.empty() && (tag_status != virt::Domain::States[info.state] && tag_status != std::to_string(info.state)))
                        continue;
                    json_res.result(res_val);
                }
            }
        }
    };

    auto networks = [&](const virt::Connection& conn) {
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
                        return error(501, "Cannot find network with a such name");
                    }
                } else if (search_key == SearchKey::by_uuid) {
                    logger.debug("Network by UUID");
                    nw = conn.networkLookupByUUIDString(key_str.c_str());
                    if (!nw) {
                        logger.error("Cannot find network with a such name");
                        return error(502, "Cannot find network with a such UUID");
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
                    return error(503, "Error occurred while getting network autostart");
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
                    return error(500, "Error occurred while getting network status");
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
                        return error(500, "Error occurred while getting network status");
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
            return error(1, "Bad X-Auth-Key");
        logger.debug("Opening connection to ", iniConfig.getConnURI());
        virt::Connection conn{iniConfig.connURI.c_str()};
        if (!conn) {
            logger.error("Failed to open connection to ", iniConfig.getConnURI());
            return error(10, "Failed to open connection to LibVirtD");
        }
        if (req.target().starts_with("/libvirt/domains"))
            return domains(std::move(conn));
        else if (req.target().starts_with("/libvirt/networks"))
            return networks(std::move(conn));
        else
            return error(2, "Bad URL");
    }();

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType, rapidjson::UTF8<>> writer(buffer);
    json_res.Accept(writer);

    return buffer;
}