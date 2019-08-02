#pragma once
#include <array>
#include <gsl/gsl>
#include "cexpr_algs.hpp"
#include "json_utils.hpp"

enum class ActionOutcome { SUCCESS, FAILURE, SKIPPED };

class DomainActionsTable {
  private:
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
            if (!val.IsString())
                return error(300, "Invalid power management value");
            const auto pm_req = val.GetString();
            using namespace std::literals;
            const auto dom_state = virt::Domain::State{dom.getInfo().state};
            if (pm_req == "shutdown"sv) {
                if (dom_state != virt::Domain::State::RUNNING)
                    return error(201, "Domain is not running");
                if (!dom.shutdown()) {
                    logger.error("Cannot shut down this domain: ", key_str);
                    return error(200, "Could not shut down the domain");
                }
                return pm_message("shutdown", "Domain is being shutdown");
            }
            if (pm_req == "destroy"sv) {
                if (dom.isActive())
                    return error(210, "Domain is not active");
                if (!dom.shutdown()) {
                    logger.error("Cannot shut down this domain: ", key_str);
                    return error(200, "Could not destroy the domain");
                }
                return pm_message("destroy", "Domain destroyed");
            }
            if (pm_req == "start"sv) {
                if (dom.isActive())
                    return error(203, "Domain is already active");
                if (!dom.create()) {
                    logger.error("Cannot start this domain: ", key_str);
                    return error(202, "Could not start the domain");
                }
                return pm_message("start", "Domain started");
            }
            if (pm_req == "reboot"sv) {
                if (dom_state != virt::Domain::State::RUNNING)
                    return error(201, "Domain is not running");
                dom.reboot();
                return pm_message("reboot", "Domain is being reboot");
            }
            if (pm_req == "reset"sv) {
                if (!dom.isActive())
                    return error(210, "Domain is not active");
                dom.reset();
                return pm_message("reset", "Domain was reset");
            }
            if (pm_req == "suspend"sv) {
                if (dom_state != virt::Domain::State::RUNNING)
                    return error(201, "Domain is not running");
                dom.suspend();
                return pm_message("suspend", "Domain suspended");
            }
            if (pm_req == "resume"sv) {
                if (dom_state != virt::Domain::State::PAUSED)
                    return error(211, "Domain is not suspended");
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