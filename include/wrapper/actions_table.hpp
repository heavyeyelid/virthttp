#pragma once
#include <array>
#include "cexpr_algs.hpp"
#include "json_utils.hpp"

enum class ActionOutcome { SUCCESS, FAILURE, SKIPPED };

class DomainActionsTable {
  private:
    using ActionHdl = ActionOutcome (*)(const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str);
    constexpr static std::array<std::string_view, 5> keys = {"state", "name", "memory", "max_memory", "autostart"};
    constexpr static std::array<ActionHdl, 5> fcns = {
        +[](const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str) -> ActionOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), ActionOutcome::FAILURE; };

            rapidjson::Value res_val;
            res_val.SetObject();

            const auto json_status = virt::Domain::State{val.GetInt()};
            if (to_integral(json_status) < 0 || to_integral(json_status) >= to_integral(virt::Domain::State::ENUM_END))
                return error(299, "Invalid state value");

            const auto dom_state = dom.getInfo().state;
            if (json_status == virt::Domain::State::SHUTOFF && dom_state == 1) {
                if (!dom.shutdown()) {
                    logger.error("Cannot shut down this VM: ", key_str);
                    return error(200, "Could not shut down the VM");
                }
                json_res.AddMember("status", 5, json_res.GetAllocator());
                rapidjson::Value msg_val{};
                msg_val.SetObject();
                msg_val.AddMember("shutdown", "Domain is shutting down", json_res.GetAllocator());

                json_res.message(msg_val);
                json_res.result(res_val);
                json_res["success"] = true;
            } else if (json_status == virt::Domain::State::RUNNING && dom.getInfo().state == 5) {
                if (!dom.resume()) {
                    logger.error("Cannot start this VM: ", key_str);
                    return error(202, "Could not start the VM");
                }
                dom.resume();
                json_res.AddMember("status", 1, json_res.GetAllocator());
                rapidjson::Value msg_val{};
                msg_val.SetObject();
                msg_val.AddMember("starting", "Domain is starting", json_res.GetAllocator());
                json_res.message(msg_val);
                json_res.result(res_val);
                json_res["success"] = true;
            } else if (json_status == virt::Domain::State::SHUTOFF && dom_state == 5) {
                return error(201, "Domain is not running");
            } else if (json_status == virt::Domain::State::RUNNING && dom_state == 1) {
                return error(203, "Domain is already running");
            } else {
                return error(204, "No status actions specified");
            }
            return ActionOutcome::SUCCESS;
        },
        +[](const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str) -> ActionOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), ActionOutcome::FAILURE; };
            if (!val.IsString())
                return error(0, "Syntax error");
            if (!dom.rename(val.GetString()))
                return error(205, "Renaming failed");
            json_res["success"] = true;
            return ActionOutcome::SUCCESS;
        },
        +[](const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str) -> ActionOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), ActionOutcome::FAILURE; };
            if (!val.IsInt())
                return error(0, "Syntax error");
            if (!dom.setMemory(val.GetInt()))
                return error(206, "Setting available memory failed");
            json_res["success"] = true;
            return ActionOutcome::SUCCESS;
        },
        +[](const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str) -> ActionOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), ActionOutcome::FAILURE; };
            if (!val.IsInt())
                return error(0, "Syntax error");
            if (!dom.setMaxMemory(val.GetInt()))
                return error(207, "Setting maximum available memory failed");
            json_res["success"] = true;
            return ActionOutcome::SUCCESS;
        },
        +[](const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str) -> ActionOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), ActionOutcome::FAILURE; };
            if (!val.IsBool())
                return error(0, "Syntax error");
            if (!dom.setAutoStart(val.GetBool()))
                return error(208, "Setting autostart policy failed");
            json_res["success"] = true;
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