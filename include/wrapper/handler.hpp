//
// Created by hugo on 09.06.19.
//

#pragma once

#include <boost/beast/http/message.hpp>
#include <rapidjson/document.h>

#include "virt_wrap.hpp"
#include "fwd.hpp"
#include "logger.hpp"
#include "urlparser.hpp"

namespace beast = boost::beast;
namespace http = beast::http;

constexpr std::string_view bsv2stdsv(boost::string_view bsv) noexcept { return {bsv.data(), bsv.length()}; }

template <class Body, class Allocator> rapidjson::StringBuffer handle_json(http::request<Body, http::basic_fields<Allocator>>&& req) {
    auto target = TargetParser{bsv2stdsv(req.target())};

    enum class SearchKey { by_name, by_uuid, none } search_key = SearchKey::none;

    rapidjson::Document json_res{};
    json_res.SetObject();
    json_res.AddMember("results", "", json_res.GetAllocator());
    json_res.AddMember("success", false, json_res.GetAllocator());
    json_res.AddMember("errors", "", json_res.GetAllocator());
    json_res.AddMember("messages", "", json_res.GetAllocator());
    auto& jResults = json_res["results"].SetArray();
    auto& jErrors = json_res["errors"].SetArray();
    auto& jMessages = json_res["messages"].SetArray();
    std::string key_str{};

    auto error = [&](int code, std::string_view msg) {
        json_res["success"] = false;
        rapidjson::Value jErrorValue{};
        jErrorValue.SetObject();
        jErrorValue.AddMember("code", code, json_res.GetAllocator());
        jErrorValue.AddMember("message", rapidjson::StringRef(msg.data(), msg.length()), json_res.GetAllocator());
        jErrors.PushBack(jErrorValue, json_res.GetAllocator());
    };

    auto getSearchKey = [&](std::string type) {
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

    auto domains = [&](virt::Connection conn) {
        if (!getSearchKey("domains"))
            return;

        if (search_key != SearchKey::none) {
            virt::Domain dom{};
            if (search_key == SearchKey::by_name) {
                logger.debug("Getting domain by name");
                dom = std::move(conn.domainLookupByName(key_str.c_str()));
                if (!dom) {
                    logger.error("Cannot find VM with name: ", key_str);
                    return error(100, "Cannot find VM with a such name");
                }
            } else if (search_key == SearchKey::by_uuid) {
                logger.debug("Getting domain by uuid");
                dom = std::move(conn.domainLookupByUUIDString(key_str.c_str()));
                if (!dom) {
                    logger.error("Cannot find VM with UUID: ", key_str);
                    return error(101, "Cannot find VM with a such UUID");
                }
            }
            switch (req.method()) {
            case http::verb::patch: {
                rapidjson::Value res_val;
                res_val.SetObject();

                rapidjson::Document json_req{};
                json_req.Parse(req.body().data());

                if (json_req["status"] == 5 && dom.getInfo().state == 1) {
                    if (!dom.shutdown()) {
                        logger.error("Cannot shut down this VM: ", key_str);
                        return error(200, "Could not shut down the VM");
                    }
                    res_val.AddMember("status", 5, json_res.GetAllocator());
                    rapidjson::Value msg_val{};
                    msg_val.SetObject();
                    msg_val.AddMember("shutdown", "Domain is shutting down", json_res.GetAllocator());

                    jMessages.PushBack(msg_val, json_res.GetAllocator());
                    jResults.PushBack(res_val, json_res.GetAllocator());
                    json_res["success"] = true;
                } else if (json_req["status"] == 1 && dom.getInfo().state == 5) {
                    if (!dom.resume()) {
                        logger.error("Cannot start this VM: ", key_str);
                        return error(202, "Could not start the VM");
                    }
                    dom.resume();
                    res_val.AddMember("status", 1, json_req.GetAllocator());
                    rapidjson::Value msg_val{};
                    msg_val.SetObject();
                    msg_val.AddMember("starting", "Domain is starting", json_res.GetAllocator());
                    jMessages.PushBack(msg_val, json_res.GetAllocator());
                    jResults.PushBack(res_val, json_res.GetAllocator());
                    json_res["success"] = true;
                } else if (json_req["status"] == 5 && dom.getInfo().state == 5) {
                    return error(201, "Domain is not running");
                } else if (json_req["status"] == 1 && dom.getInfo().state == 1) {
                    return error(203, "Domain is already running");
                } else {
                    return error(204, "No actions specified");
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

                jResults.PushBack(res_val, json_res.GetAllocator());
                json_res["success"] = true;
            } break;
            default: {
            }
            }
        } else {
            if (req.method() == http::verb::get) {
                logger.debug("Listing all domains");
                const auto [tag_name, tag_uuid, tag_status] = std::make_tuple(target["name"], target["uuid"], target["status"]);
                for (const auto& dom : conn.listAllDomains()) {
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
                    if (!tag_status.empty() ||
                        (tag_status != virt::Domain::States[info.state] && tag_status != std::to_string(info.state)))
                        continue;
                    jResults.PushBack(res_val, json_res.GetAllocator());
                }
                json_res["success"] = true;
            }
        }
    };

    auto networks = [&](virt::Connection conn) {
        if (!getSearchKey("networks"))
            return;
        switch (req.method()) {
        case http::verb::get: {
            if (search_key != SearchKey::none) {
                virt::Network nw{};
                if (search_key == SearchKey::by_name) {
                    logger.debug("Network by name");
                } else if (search_key == SearchKey::by_uuid) {
                    logger.debug("Network by UUID");
                }
            } else {
                logger.debug("Listing all networks - WIP"); // WIP
                for (const auto& nw : conn.extractAllNetworks()) {
                    TFE nwActive = nw.isActive();
                    rapidjson::Value jsonActive;
                    if (!nwActive.err()) {
                        if (nwActive)
                            jsonActive.SetBool(true);
                        else
                            jsonActive.SetBool(false);
                    } else {
                        logger.error("Error occurred while getting networks");
                        return error(250, "Error occurred while getting networks");
                    }
                    rapidjson::Value nw_json;
                    nw_json.SetObject();
                    nw_json.AddMember("name", rapidjson::Value(nw.getName(), json_res.GetAllocator()), json_res.GetAllocator());
                    nw_json.AddMember("active", jsonActive, json_res.GetAllocator());
                    nw_json.AddMember("uuid", nw.extractUUIDString(), json_res.GetAllocator());
                    jResults.PushBack(nw_json, json_res.GetAllocator());
                }
                json_res["success"] = true;
            }
        } break;
        default: {
        }
        }
    };

    [&] {
        if (req["X-Auth-Key"] != iniConfig.http_auth_key) {
            return error(1, "Bad X-Auth-Key");
        }
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