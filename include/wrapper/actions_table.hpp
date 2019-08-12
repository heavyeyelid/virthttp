#pragma once
#include <array>
#include <functional>
#include <string_view>
#include <gsl/gsl>
#include "cexpr_algs.hpp"
#include "json_utils.hpp"
#include "depends.hpp"
#include "utils.hpp"

#define PM_LIFT(mem_fn) [&](auto... args) { return mem_fn(args...); }
#define PM_PREREQ(...) [&] { __VA_ARGS__ return ActionOutcome::SUCCESS; }

using namespace std::literals;

using ActionOutcome = DependsOutcome;

constexpr auto action_scope = [](auto&&... actions) {
    using Arr = std::array<std::function<ActionOutcome()>, sizeof...(actions)>; // pray for SFO ; wait for expansion statements
    for (auto&& action : Arr{actions...}) {
        if (const auto ao = action(); ao != ActionOutcome::SKIPPED)
            return ao;
    }
    return ActionOutcome::SKIPPED;
};

class DomainActionsTable {
  private:
    template <typename Flag, typename Flags>
    constexpr static const auto getFlag = [](const rapidjson::Value& json_flag, auto error) {
        if (auto v = Flags{}[std::string_view{json_flag.GetString(), json_flag.GetStringLength()}]; v)
            return std::optional{*v};
        return error(301), std::optional<Flag>{std::nullopt};
    };
    template <class F, class Fs> static std::optional<F> getCombinedFlags(const rapidjson::Value& json_flag, JsonRes& json_res) noexcept {
        auto error = [&](auto... args) { return json_res.error(args...), std::nullopt; };
        auto getFlag = DomainActionsTable::getFlag<F, Fs>;

        F flagset{};
        if (json_flag.IsArray()) {
            const auto json_arr = json_flag.GetArray();
            if constexpr (test_sfinae([](auto f) -> decltype(f | f) {}, F{})) {
                for (const auto& json_str : json_arr) {
                    const auto v = getFlag(json_str, error);
                    if (!v)
                        return std::nullopt;
                    flagset |= *v;
                }
            } else {
                if (json_arr.Size() > 1)
                    return error(301);
                return {json_arr.Empty() ? F{} : getFlag(json_arr[0], error)};
            }
        } else if (json_flag.IsString()) {
            const auto v = getFlag(json_flag, error);
            if (!v)
                return std::nullopt;
            flagset = *v;
        }
        return {flagset};
    }

    using ActionHdl = ActionOutcome (*)(const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str);
    constexpr static std::array<std::string_view, 5> keys = {"power_mgt", "name", "memory", "max_memory", "autostart"};
    constexpr static std::array<ActionHdl, 5> fcns = {
        +[](const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str) -> ActionOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), ActionOutcome::FAILURE; };
            auto pm_message = [&](gsl::czstring<> name, gsl::czstring<> value) {
                rapidjson::Value msg_val{};
                msg_val.SetObject();
                msg_val.AddMember(rapidjson::StringRef(name), rapidjson::StringRef(value), json_res.GetAllocator());
                json_res.message(msg_val);
                return ActionOutcome::SUCCESS;
            };

            constexpr auto getShutdownFlag = getFlag<virt::Domain::ShutdownFlag, virt::Domain::ShutdownFlagsC>;

            const rapidjson::Value* json_flag = nullptr;
            gsl::czstring<> pm_req{};

            if (val.IsString()) {
                pm_req = val.GetString();
            } else if (val.IsObject()) {
                auto it_req = val.FindMember("request");
                auto it_flags = val.FindMember("type");
                if (it_req == val.MemberEnd())
                    return error(300);
                if (it_flags != val.MemberEnd())
                    json_flag = &it_flags->value;
                pm_req = it_req->value.GetString();
            } else {
                return error(300);
            }

            const auto dom_state = virt::Domain::State{dom.getInfo().state};

            const auto pm_hdl = [&](gsl::czstring<> req_tag, auto flags, auto mem_fcn, int errc, gsl::czstring<> pm_msg, auto prereqs) {
                using Flag = typename decltype(flags)::First;
                using FlagsC = typename decltype(flags)::Second;
                return [=, &json_flag, &json_res]() {
                    const auto local_error = [&] {
                        const auto err_msg = error_messages[errc];
                        logger.error(err_msg, " :", key_str);
                        return error(errc);
                    };

                    if (pm_req == std::string_view{req_tag}) {
                        if (prereqs() == ActionOutcome::FAILURE)
                            return ActionOutcome::FAILURE;
                        if (json_flag) {
                            if constexpr (test_sfinae([](auto f) -> std::enable_if_t<!std::is_same_v<decltype(f), Empty>> {}, Flag{})) {
                                constexpr const auto getFlags = getCombinedFlags<Flag, FlagsC>;
                                const auto o_flagset = getFlags(*json_flag, json_res);
                                if (!o_flagset)
                                    return ActionOutcome::FAILURE;
                                if (const auto flagset = *o_flagset; !mem_fcn(flagset))
                                    return local_error();
                            } else
                                return error(301);
                        } else {
                            if constexpr (test_sfinae([](auto f) { f(); }, mem_fcn)) {
                                if (!mem_fcn())
                                    local_error();
                            } else
                                return error(301);
                        }
                        return pm_message(req_tag, pm_msg);
                    }
                    return ActionOutcome::SKIPPED;
                };
            };
            constexpr auto no_flags = tp<Empty, Empty>;
            return action_scope(
                pm_hdl("shutdown", tp<virt::Domain::ShutdownFlag, virt::Domain::ShutdownFlagsC>, PM_LIFT(dom.shutdown), 200,
                       "Domain is being shutdown", PM_PREREQ(if (dom_state != virt::Domain::State::RUNNING) return error(201);)),
                pm_hdl("destroy", tp<virt::Domain::DestroyFlag, virt::Domain::DestroyFlagsC>, PM_LIFT(dom.destroy), 209, "Domain destroyed",
                       PM_PREREQ(if (!dom.isActive()) return error(210);)),
                pm_hdl("start", tp<virt::Domain::CreateFlag, virt::Domain::CreateFlagsC>, PM_LIFT(dom.create), 202, "Domain started",
                       PM_PREREQ(if (dom.isActive()) return error(203);)),
                pm_hdl("reboot", tp<virt::Domain::ShutdownFlag, virt::Domain::ShutdownFlagsC>, PM_LIFT(dom.reboot), 213, "Domain is being rebooted",
                       PM_PREREQ(if (dom_state != virt::Domain::State::RUNNING) return error(201);)),
                pm_hdl("reset", no_flags, PM_LIFT(dom.reset), 214, "Domain was reset", PM_PREREQ(if (!dom.isActive()) return error(210);)),
                pm_hdl("suspend", no_flags, PM_LIFT(dom.suspend), 215, "Domain suspended",
                       PM_PREREQ(if (dom_state != virt::Domain::State::RUNNING) return error(201);)),
                pm_hdl("resume", no_flags, PM_LIFT(dom.resume), 212, "Domain resumed",
                       PM_PREREQ(if (dom_state != virt::Domain::State::PAUSED) return error(211);)),
                [&]() { return error(300); });
        },
        +[](const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str) -> ActionOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), ActionOutcome::FAILURE; };
            if (!val.IsString())
                return error(0);
            if (!dom.rename(val.GetString()))
                return error(205);
            return ActionOutcome::SUCCESS;
        },
        +[](const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str) -> ActionOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), ActionOutcome::FAILURE; };
            if (!val.IsInt())
                return error(0);
            if (!dom.setMemory(val.GetInt()))
                return error(206);
            return ActionOutcome::SUCCESS;
        },
        +[](const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str) -> ActionOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), ActionOutcome::FAILURE; };
            if (!val.IsInt())
                return error(0);
            if (!dom.setMaxMemory(val.GetInt()))
                return error(207);
            return ActionOutcome::SUCCESS;
        },
        +[](const rapidjson::Value& val, JsonRes& json_res, virt::Domain& dom, const std::string& key_str) -> ActionOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), ActionOutcome::FAILURE; };
            if (!val.IsBool())
                return error(0);
            if (!dom.setAutoStart(val.GetBool()))
                return error(208);
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