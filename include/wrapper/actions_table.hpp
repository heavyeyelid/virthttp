#pragma once
#include <array>
#include <gsl/gsl>
#include "cexpr_algs.hpp"
#include "json_utils.hpp"

enum class ActionOutcome { SUCCESS, FAILURE, SKIPPED };

class DomainActionsTable {
  private:
    template <class Flag, class Flags, class Err> constexpr auto genGetFlag(const Flags& flags, Err& error) noexcept {
        return [&](const rapidjson::Value& json_flag) -> std::optional<Flag> {
            if (auto v = flags[std::string_view{json_flag.GetString(), json_flag.GetStringLength()}]; v)
                return std::optional{*v};
            return error(301, "Invalid flag"), std::nullopt;
        };
    }
    using ActionHdl = ActionOutcome (*)(const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str);
    constexpr static std::array<std::string_view, 5> keys = {"power_mgt", "name", "memory", "max_memory", "autostart"};
    constexpr static std::array<ActionHdl, 5> fcns = {
        +[](const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str) -> ActionOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), ActionOutcome::FAILURE; };
            auto pm_message = [&](const std::string& name, const std::string& value) {
                rapidjson::Value msg_val{};
                msg_val.SetObject();
                msg_val.AddMember(rapidjson::Value(name, json_res.GetAllocator()), rapidjson::Value(value, json_res.GetAllocator()),
                                  json_res.GetAllocator());
                json_res.message(msg_val);
                return ActionOutcome::SUCCESS;
            };
            auto getShutdownFlag = [&](const std::string& flag) -> std::optional<virt::Domain::ShutdownFlag> {
                virt::Domain::ShutdownFlag shutdownFlag;
                if (std::all_of(flag.cbegin(), flag.cend(), [&](auto c) { return std::isdigit(c); }))
                    shutdownFlag = virt::Domain::ShutdownFlag(std::stol(std::string{flag}));
                else if (auto v = virt::Domain::ShutdownFlags[flag]; v)
                    shutdownFlag = *v;
                else
                    return error(301, "Invalid flag"), std::nullopt;
                return std::optional{shutdownFlag};
            };

            std::string pm_req{}, json_flag{};
            bool isFlag{};

            if (val.IsString()) {
                pm_req = val.GetString();
                isFlag = false;
            } else if (val.IsObject()) {
                auto it_req = val.FindMember("request");
                auto it_flags = val.FindMember("type");
                if (it_req == val.MemberEnd())
                    return error(300, "Invalid power management value");
                if (it_flags != val.MemberEnd()) {
                    auto v = &it_flags->value;
                    isFlag = true;
                    if (v->IsString())
                        json_flag = it_flags->value.GetString();
                    else if (v->IsArray())
                        // Stuff if flag is an array
                        ;
                    else
                        return error(301, "Invalid flag");
                }
                pm_req = it_req->value.GetString();
            } else {
                return error(300, "Invalid power management value");
            }
            using namespace std::literals;
            const auto dom_state = virt::Domain::State{dom.getInfo().state};

            // Start specific flag handling to make generic
            if (pm_req == "shutdown"sv) {
                if (dom_state != virt::Domain::State::RUNNING)
                    return error(201, "Domain is not running");
                if (isFlag) {
                    virt::Domain::ShutdownFlag flagset{};
                    auto it_flags = val.FindMember("type");
                    if (it_flags != val.MemberEnd()) {
                        const auto& fl = it_flags->value;
                        if (fl.IsArray()) {
                            auto json_arr = it_flags->value.GetArray();
                            logger.debug("Flag is an array");
                            for (const auto& json_str : json_arr) {
                                auto v = getShutdownFlag(json_str.GetString());
                                if (!v)
                                    return error(301, "Invalid flag");
                                flagset |= *v;
                            }
                        } else if (fl.IsString()) {
                            flagset = *getShutdownFlag(fl.GetString());
                        }
                    }

                    if (!dom.shutdown(flagset)) {
                        logger.error("Cannot shutdown this domain: ", key_str);
                        return error(200, "Could not shutdown the domain");
                    }
                } else {
                    if (!dom.shutdown()) {
                        logger.error("Cannot shutdown this domain: ", key_str);
                        return error(200, "Could not shutdown the domain");
                    }
                }
                return pm_message("shutdown", "Domain is being shutdown");
            }
            // End

            if (pm_req == "destroy"sv) {
                if (!dom.isActive())
                    return error(210, "Domain is not active");
                if (isFlag) {
                    virt::Domain::DestroyFlag destroyFlag;
                    if (std::all_of(json_flag.cbegin(), json_flag.cend(), [&](auto c) { return std::isdigit(c); }))
                        destroyFlag = virt::Domain::DestroyFlag(std::stol(std::string{json_flag}));
                    else if (auto v = virt::Domain::DestroyFlags[json_flag]; v)
                        destroyFlag = *v;
                    else
                        return error(301, "Invalid flag");
                    if (!dom.destroy(destroyFlag)) {
                        logger.error("Cannot destroy this domain: ", key_str);
                        return error(209, "Could not destroy the domain");
                    }
                } else {
                    if (!dom.shutdown()) {
                        logger.error("Cannot destroy this domain: ", key_str);
                        return error(209, "Could not destroy the domain");
                    }
                }
                return pm_message("destroy", "Domain destroyed");
            }
            if (pm_req == "start"sv) {
                if (dom.isActive())
                    return error(203, "Domain is already active");
                if (isFlag) {
                    logger.debug("Flag is an array");
                    virt::Domain::CreateFlag createFlag;
                    if (std::all_of(json_flag.cbegin(), json_flag.cend(), [&](auto c) { return std::isdigit(c); }))
                        createFlag = virt::Domain::CreateFlag(std::stol(std::string{json_flag}));
                    else if (auto v = virt::Domain::CreateFlags[json_flag]; v)
                        createFlag = *v;
                    else
                        return error(301, "Invalid flag");
                    if (!dom.create(createFlag)) {
                        logger.error("Cannot start this domain: ", key_str);
                        return error(202, "Could not start the domain");
                    }
                } else {
                    if (!dom.create()) {
                        logger.error("Cannot start this domain: ", key_str);
                        return error(202, "Could not start the domain");
                    }
                }
                return pm_message("start", "Domain started");
            }
            if (pm_req == "reboot"sv) {
                if (dom_state != virt::Domain::State::RUNNING)
                    return error(201, "Domain is not running");
                if (isFlag) {
                    const auto v = getShutdownFlag(json_flag);
                    if (!v)
                        return error(301, "Invalid flag");
                    dom.reboot(*v);
                } else
                    dom.reboot();
                return pm_message("reboot", "Domain is being reboot");
            }
            if (pm_req == "reset"sv) {
                if (!dom.isActive())
                    return error(210, "Domain is not active");
                if (isFlag)
                    return error(301, "Invalid flag");
                dom.reset();
                return pm_message("reset", "Domain was reset");
            }
            if (pm_req == "suspend"sv) {
                if (dom_state != virt::Domain::State::RUNNING)
                    return error(201, "Domain is not running");
                if (isFlag)
                    return error(301, "Invalid flag");
                dom.suspend();
                return pm_message("suspend", "Domain suspended");
            }
            if (pm_req == "resume"sv) {
                if (dom_state != virt::Domain::State::PAUSED)
                    return error(211, "Domain is not suspended");
                if (isFlag)
                    return error(301, "Invalid flag");
                if (!dom.resume()) {
                    logger.error("Cannot resume this domain: ", key_str);
                    return error(212, "Cannot resume the domain");
                }
                return pm_message("resume", "Domain resumed");
            }
            return error(300, "Invalid power management value");
        },
        +[](const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str) -> ActionOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), ActionOutcome::FAILURE; };
            if (!val.IsString())
                return error(0, "Syntax error");
            if (!dom.rename(val.GetString()))
                return error(205, "Renaming failed");
            return ActionOutcome::SUCCESS;
        },
        +[](const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str) -> ActionOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), ActionOutcome::FAILURE; };
            if (!val.IsInt())
                return error(0, "Syntax error");
            if (!dom.setMemory(val.GetInt()))
                return error(206, "Setting available memory failed");
            return ActionOutcome::SUCCESS;
        },
        +[](const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str) -> ActionOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), ActionOutcome::FAILURE; };
            if (!val.IsInt())
                return error(0, "Syntax error");
            if (!dom.setMaxMemory(val.GetInt()))
                return error(207, "Setting maximum available memory failed");
            return ActionOutcome::SUCCESS;
        },
        +[](const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str) -> ActionOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), ActionOutcome::FAILURE; };
            if (!val.IsBool())
                return error(0, "Syntax error");
            if (!dom.setAutoStart(val.GetBool()))
                return error(208, "Setting autostart policy failed");
            return ActionOutcome::SUCCESS;
        },
    };

  public:
    constexpr ActionHdl operator[](std::string_view sv) const {
        const auto it = cexpr::find(keys.begin(), keys.end(), sv);
        if (it == keys.end())
            return nullptr;
        const auto idx = std::distance(keys.begin(), it);
        return fcns[idx];
    }
} constexpr static const domain_actions_table;

class DomainQueryTable {
  private:
    using ActionHdl = ActionOutcome (*)(const rapidjson::Value& val, JsonRes& json_res, const virt::Domain& dom, const std::string& key_str);
    constexpr static std::array<std::string_view, 0> keys = {};
    constexpr static std::array<ActionHdl, 0> fcns = {};

  public:
    constexpr ActionHdl operator[](std::string_view sv) const {
        const auto it = cexpr::find(keys.begin(), keys.end(), sv);
        if (it == keys.end())
            return nullptr;
        const auto idx = std::distance(keys.begin(), it);
        return fcns[idx];
    }
} constexpr static const domain_query_table;