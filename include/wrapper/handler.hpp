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

enum class SearchKey { by_name, by_uuid, none } search_key = SearchKey::none;

constexpr std::string_view bsv2stdsv(boost::string_view bsv) noexcept { return {bsv.data(), bsv.length()}; }

virt::Domain getDomain(const virt::Connection& conn, const std::string& dom_str) {
    if (search_key == SearchKey::by_name) {
        logger.debug("Getting domain by name");
        return (conn.domainLookupByName(dom_str.c_str()));
    } else if (search_key == SearchKey::by_uuid) {
        logger.debug("Getting domain by uuid");
        return conn.domainLookupByUUIDString(dom_str.c_str());
    }
    return virt::Domain{};
}

template <class Body, class Allocator> rapidjson::StringBuffer handle_json(http::request<Body, http::basic_fields<Allocator>>&& req) {
    //    auto target = TargetParser{bsv2stdsv(req.target())};

    rapidjson::Document json_res{};
    json_res.SetObject();
    json_res.AddMember("results", "", json_res.GetAllocator());
    json_res.AddMember("success", false, json_res.GetAllocator());
    json_res.AddMember("errors", "", json_res.GetAllocator());
    json_res.AddMember("messages", "", json_res.GetAllocator());
    auto& jResults = json_res["results"].SetArray();
    auto& jErrors = json_res["errors"].SetArray();
    auto& jMessages = json_res["messages"].SetArray();

    auto error = [&](int code, std::string_view msg) -> void {
        json_res["success"] = false;
        rapidjson::Value jErrorValue{};
        jErrorValue.SetObject();
        jErrorValue.AddMember("code", code, json_res.GetAllocator());
        jErrorValue.AddMember("message", rapidjson::StringRef(msg.data(), msg.length()), json_res.GetAllocator());
        jErrors.PushBack(jErrorValue, json_res.GetAllocator());
    };

    [&]() {
        if (req["X-Auth-Key"] != iniConfig.http_auth_key) {
            return error(1, "Bad X-Auth-Key");
        }
        if (!req.target().starts_with("/libvirt/domains")) {
            return error(2, "Bad URL");
        }
        logger.debug("Opening connection to ", iniConfig.getConnURI());
        virt::Connection conn{iniConfig.connURI.c_str()};
        if (!conn) {
            logger.error("Failed to open connection to ", iniConfig.getConnURI());
            return error(10, "Failed to open connection to LibVirtD");
        }

        std::string dom_str{};
        if (req.target().starts_with("/libvirt/domains/by-uuid/")) {
            dom_str = std::string{req.target().substr(25)};
            search_key = SearchKey::by_uuid;
        } else if (req.target().starts_with("/libvirt/domains/by-name/")) {
            dom_str = std::string{req.target().substr(25)};
            search_key = SearchKey::by_name;
        } else if (!req.target().substr(16).empty() && !req.target().substr(17).empty()) {
            dom_str = std::string{req.target().substr(17)};
            search_key = SearchKey::by_name;
        } else
            search_key = SearchKey::none;

        if (search_key != SearchKey::none) {
            virt::Domain dom = getDomain(conn, dom_str);
            if (!dom) {
                if (search_key == SearchKey::by_name) {
                    logger.error("Cannot find VM with name: ", dom_str);
                    return error(100, "Cannot find VM with a such name");
                } else if (search_key == SearchKey::by_uuid) {
                    logger.error("Cannot find VM with UUID: ", dom_str);
                    return error(101, "Cannot find VM with a such UUID");
                }
            }

            //            if (search_key == SearchKey::by_name) {
            //                logger.debug("Getting domain by name");
            //                dom = std::move(conn.domainLookupByName(dom_str.c_str()));
            //                if (!dom) {
            //                    logger.error("Cannot find VM with name: ", dom_str);
            //                    return error(100, "Cannot find VM with a such name");
            //                }
            //            } else if (search_key == SearchKey::by_uuid) {
            //                logger.debug("Getting domain by uuid");
            //                dom = std::move(conn.domainLookupByUUIDString(dom_str.c_str()));
            //                if (!dom) {
            //                    logger.error("Cannot find VM with UUID: ", dom_str);
            //                    return error(101, "Cannot find VM with a such UUID");
            //                }
            //            }
            switch (req.method()) {
            case http::verb::patch: {
                rapidjson::Value res_val;
                res_val.SetObject();

                rapidjson::Document json_req{};
                json_req.Parse(req.body().data());

                if (json_req["status"] == 5 && dom.getInfo().state == 1) {
                    if (!dom.shutdown()) {
                        logger.error("Cannot shut down this VM: ", dom_str);
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
                        logger.error("Cannot start this VM: ", dom_str);
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
                    error(201, "Domain is not running");
                } else if (json_req["status"] == 1 && dom.getInfo().state == 1) {
                    error(203, "Domain is already running");
                } else {
                    error(204, "No actions specified");
                }
            } break;
            case http::verb::get: {
                rapidjson::Value res_val;
                res_val.SetObject();
                const auto [state, max_mem, memory, nvirt_cpu, cpu_time] = dom.getInfo();
                const auto os_type = dom.getOSType();
                res_val.AddMember("name", rapidjson::Value(dom.getName(), json_res.GetAllocator()), json_res.GetAllocator());
                res_val.AddMember("uuid", dom.getUUIDString(), json_res.GetAllocator());
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
                for (const auto& dom : conn.listAllDomains()) {
                    rapidjson::Value res_val;
                    res_val.SetObject();
                    const auto info = dom.getInfo();
                    res_val.AddMember("name", rapidjson::Value(dom.getName(), json_res.GetAllocator()), json_res.GetAllocator());
                    res_val.AddMember("uuid", dom.getUUIDString(), json_res.GetAllocator());
                    res_val.AddMember("status", rapidjson::StringRef(virt::Domain::States[info.state]), json_res.GetAllocator());
                    jResults.PushBack(res_val, json_res.GetAllocator());
                    json_res["success"] = true;
                }
            }
        }
    }();

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType, rapidjson::UTF8<>> writer(buffer);
    json_res.Accept(writer);

    return buffer;
}