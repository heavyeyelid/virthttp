//
// Created by hugo on 09.06.19.
//

#pragma once

#include <rapidjson/document.h>
#include <boost/beast/http/message.hpp>

#include "logger.hpp"
#include "urlparser.hpp"
#include "fwd.hpp"
#include "virt_wrap.hpp"

namespace beast = boost::beast;
namespace http = beast::http;

constexpr std::string_view bsv2stdsv(boost::string_view bsv) noexcept {
    return {bsv.data(), bsv.length()};
}

template<class Body, class Allocator>
rapidjson::StringBuffer getResult(http::request<Body, http::basic_fields<Allocator>> &&req) {
    auto path = TargetParser{bsv2stdsv(req.target())};

    rapidjson::Document json_res{};
    json_res.SetObject();
    json_res.AddMember("results", "", json_res.GetAllocator());
    json_res.AddMember("success", false, json_res.GetAllocator());
    json_res.AddMember("errors", "", json_res.GetAllocator());
    json_res.AddMember("messages", "", json_res.GetAllocator());
    auto &jResults = json_res["results"].SetArray();
    auto &jErrors = json_res["errors"].SetArray();
    auto &jMessages = json_res["messages"].SetArray();

    if (req["X-Auth-Key"] == iniConfig.http_auth_key) {
        if (req.target().starts_with("/libvirt/domains")) {
            logger.debug("Opening connection to ", iniConfig.getConnURI());
            try {
                virt::Connection conn{iniConfig.connURI.data()};

                if (req.target().starts_with("/libvirt/domains/by-name")) {
                    logger.debug("Getting domain information by name");
                    const std::string substr{req.target().substr(25)};
                    try {
                        virt::Domain dom = conn.domainLookupByName(substr.c_str());
                        rapidjson::Value res_val{};
                        res_val.SetObject();

                        if (req.method() == http::verb::patch) {
                            rapidjson::Document json_req{};
                            json_req.Parse(req.body().data());
                            if (json_req["status"] == 5 && dom.getInfo().state == 1) {
                                try {
                                    dom.shutdown();
                                    res_val.AddMember("status", 5, json_res.GetAllocator());
                                    rapidjson::Value msg_val{};
                                    msg_val.SetObject();
                                    msg_val.AddMember("shutdown", "Domain is shutting down", json_res.GetAllocator());

                                    jMessages.PushBack(msg_val, json_res.GetAllocator());
                                    jResults.PushBack(res_val, json_res.GetAllocator());
                                    json_res["success"] = true;

                                } catch (std::exception &e) {
                                    json_res["success"] = false;
                                    rapidjson::Value err_val{};

                                    err_val.SetObject();
                                    err_val.AddMember("code", 200, json_res.GetAllocator());
                                    err_val.AddMember("message", "Cannot shut down the VM", json_res.GetAllocator());
                                    jErrors.PushBack(err_val, json_res.GetAllocator());

                                    logger.error("Cannot shut down this VM: ", substr);
                                    logger.debug(e.what());
                                }
                            } else if (json_req["status"] == 1 && dom.getInfo().state == 5) {
                                try {
                                    dom.resume();
                                    res_val.AddMember("status", 1, json_req.GetAllocator());
                                    rapidjson::Value msg_val{};
                                    msg_val.SetObject();
                                    msg_val.AddMember("starting", "Domain is starting", json_res.GetAllocator());
                                    jMessages.PushBack(msg_val, json_res.GetAllocator());
                                    jResults.PushBack(res_val, json_res.GetAllocator());
                                    json_res["success"] = true;
                                } catch (std::exception &e) {
                                    json_res["success"] = false;
                                    rapidjson::Value err_val{};

                                    err_val.SetObject();
                                    err_val.AddMember("code", 202, json_res.GetAllocator());
                                    err_val.AddMember("message", "Cannot start the VM", json_res.GetAllocator());
                                    jErrors.PushBack(err_val, json_res.GetAllocator());

                                    logger.error("Cannot start this VM: ", substr);
                                    logger.debug(e.what());
                                }
                            } else if (json_req["status"] == 5 && dom.getInfo().state == 5) {
                                json_res["success"] = false;
                                rapidjson::Value err_val{};
                                err_val.SetObject();

                                err_val.AddMember("code", 201, json_res.GetAllocator());
                                err_val.AddMember("message", "Domain is not running", json_res.GetAllocator());
                                jErrors.PushBack(err_val, json_res.GetAllocator());
                            } else if (json_req["status"] == 1 && dom.getInfo().state == 1) {
                                json_res["success"] = false;
                                rapidjson::Value err_val{};
                                err_val.SetObject();

                                err_val.AddMember("code", 203, json_res.GetAllocator());
                                err_val.AddMember("message", "Domain is already running", json_res.GetAllocator());
                                jErrors.PushBack(err_val, json_res.GetAllocator());
                            } else {
                                json_res["success"] = false;
                                rapidjson::Value err_val{};
                                err_val.SetObject();

                                err_val.AddMember("code", 204, json_res.GetAllocator());
                                err_val.AddMember("message", "No actions specified", json_res.GetAllocator());
                                jErrors.PushBack(err_val, json_res.GetAllocator());
                            }
                        }

                        if (req.method() == http::verb::get) {
                            const std::string name{dom.getName(), std::strlen(dom.getName())};
                            res_val.AddMember("name", name, json_res.GetAllocator());
                            res_val.AddMember("uuid", dom.getUUIDString(), json_res.GetAllocator());
                            res_val.AddMember("status", dom.getInfo().state, json_res.GetAllocator());
                            //                            res_val.AddMember("os", dom.getOSType(), json_res.GetAllocator());
                            res_val.AddMember("ram", dom.getInfo().memory, json_res.GetAllocator());
                            res_val.AddMember("ram_max", dom.getInfo().maxMem, json_res.GetAllocator());
                            res_val.AddMember("cpu", dom.getInfo().nrVirtCpu, json_res.GetAllocator());

                            jResults.PushBack(res_val, json_res.GetAllocator());
                            json_res["success"] = true;
                        }
                    } catch (const std::exception &e) {
                        json_res["success"] = false;
                        rapidjson::Value err_val{};
                        err_val.SetObject();
                        err_val.AddMember("code", 100, json_res.GetAllocator());
                        err_val.AddMember("message", "Cannot find VM with a such name", json_res.GetAllocator());
                        jErrors.PushBack(err_val, json_res.GetAllocator());

                        logger.error("Cannot find VM with name: ", substr);
                        logger.debug(e.what());
                    }
                } else {
                    if (req.method() == http::verb::get) {
                        for (const auto &dom : conn.listAllDomains()) {
                            rapidjson::Value res_val{};
                            res_val.SetObject();

                            const std::string name{dom.getName(), std::strlen(dom.getName())};
                            res_val.AddMember("name", name, json_res.GetAllocator());
                            res_val.AddMember("uuid", dom.getUUIDString(), json_res.GetAllocator());
                            res_val.AddMember("status", dom.getInfo().state, json_res.GetAllocator());
                            jResults.PushBack(res_val, json_res.GetAllocator());
                            json_res["success"] = true;

                            logger.debug(name);
                            logger.debug(dom.getUUIDString());
                            logger.debug(static_cast<int>(dom.getInfo().state));
                        }
                    }
                }
            } catch (const std::exception &e) {
                json_res["success"] = false;
                rapidjson::Value err_val{};
                err_val.SetObject();
                err_val.AddMember("code", 10, json_res.GetAllocator());
                err_val.AddMember("message", "Failed to open connection to LibVirtD", json_res.GetAllocator());
                jErrors.PushBack(err_val, json_res.GetAllocator());
                logger.error("Failed to open connection to ", iniConfig.getConnURI());
                logger.debug(e.what());
            }
        } else {
            json_res["success"] = false;
            rapidjson::Value jErrorValue{};
            jErrorValue.SetObject();
            jErrorValue.AddMember("code", 2, json_res.GetAllocator());
            jErrorValue.AddMember("message", "Bad URL", json_res.GetAllocator());
            jErrors.PushBack(jErrorValue, json_res.GetAllocator());
        }
    } else {
        json_res["success"] = false;
        rapidjson::Value jErrorValue{};
        jErrorValue.SetObject();
        jErrorValue.AddMember("code", 1, json_res.GetAllocator());
        jErrorValue.AddMember("message", "Bad X-Auth-Key", json_res.GetAllocator());
        jErrors.PushBack(jErrorValue, json_res.GetAllocator());
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType, rapidjson::UTF8<>> writer(buffer);
    json_res.Accept(writer);

    return buffer;
}