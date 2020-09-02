//
// Created by hugo on 5/13/20.
//

#pragma once

#include "virt_wrap/Domain.hpp"
#include "actions_table.hpp"

using DomainActionsHdl = DependsOutcome (*)(const boost::json::value& val, JsonRes& json_res, virt::Domain& dom);
class DomainActionsTable : public NamedCallTable<DomainActionsTable, DomainActionsHdl> {
  private:
    friend NamedCallTable<DomainActionsTable, DomainActionsHdl>;

    using Hdl = DomainActionsHdl;

    constexpr static std::array<std::string_view, 7> keys = {"power_mgt", "name", "memory", "max_memory", "autostart", "send_signal", "send_keys"};
    constexpr static std::array<Hdl, 7> fcns = {
        +[](const boost::json::value& val, JsonRes& json_res, virt::Domain& dom) -> DependsOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), DependsOutcome::FAILURE; };
            auto pm_message = [&](gsl::czstring<> name, gsl::czstring<> value) {
                json_res.message(boost::json::object{{name, value}});
                return DependsOutcome::SUCCESS;
            };

            constexpr auto getShutdownFlag = getFlag<virt::enums::domain::ShutdownFlag>;

            const boost::json::value* json_flag = nullptr;
            gsl::czstring<> pm_req{};

            if (val.is_string()) {
                pm_req = val.get_string().c_str();
            } else if (val.is_object()) {
                const auto& obj = val.get_object();
                auto it_req = obj.find("request");
                auto it_flags = obj.find("type");
                if (it_req == obj.end())
                    return error(300);
                if (it_flags != obj.end())
                    json_flag = &it_flags->value();
                pm_req = it_req->value().get_string().c_str();
            } else {
                return error(300);
            }

            const auto dom_state = virt::enums::domain::State{EHTag{}, dom.getInfo().state}; // Verified use of EHTag

            const auto pm_hdl = [&](gsl::czstring<> req_tag, auto flag_ti, auto mem_fcn, int errc, gsl::czstring<> pm_msg, auto prereqs) {
                using Flag = typename decltype(flag_ti)::type;
                return [=, &json_flag, &json_res]() {
                    const auto mem_fcn_passthru = [=](auto... args) constexpr { return mem_fcn(args...); }; // WA GCC
                    const auto local_error = [&] {
                        logger.error(error_messages[errc]);
                        return error(errc);
                    };

                    if (pm_req == std::string_view{req_tag}) {
                        if (prereqs() == DependsOutcome::FAILURE)
                            return DependsOutcome::FAILURE;
                        if (json_flag) {
                            if constexpr (test_sfinae([](auto f) -> std::enable_if_t<!std::is_same_v<decltype(f), Empty>> {}, Flag{})) {
                                constexpr const auto getFlags = getCombinedFlags<Flag>;
                                const auto o_flagset = getFlags(*json_flag, json_res);
                                if (!o_flagset)
                                    return DependsOutcome::FAILURE;
                                if (const auto flagset = *o_flagset; !mem_fcn(flagset))
                                    return local_error();
                            } else
                                return error(301);
                        } else {
                            if constexpr (test_sfinae([](auto f) { f(); }, mem_fcn_passthru)) {
                                if (!mem_fcn())
                                    return local_error();
                            } else
                                return error(301);
                        }
                        return pm_message(req_tag, pm_msg);
                    }
                    return DependsOutcome::SKIPPED;
                };
            };
            constexpr auto no_flags = ti<Empty>;
            return action_scope(
                pm_hdl("shutdown", ti<virt::enums::domain::ShutdownFlag>, PM_LIFT(dom.shutdown), 200, "Domain is being shutdown",
                       PM_PREREQ(if (dom_state != virt::enums::domain::State::RUNNING) return error(201);)),
                pm_hdl("destroy", ti<virt::enums::domain::DestroyFlag>, PM_LIFT(dom.destroy), 209, "Domain destroyed",
                       PM_PREREQ(if (!dom.isActive()) return error(210);)),
                pm_hdl("start", ti<virt::enums::domain::CreateFlag>, PM_LIFT(dom.create), 202, "Domain started",
                       PM_PREREQ(if (dom.isActive()) return error(203);)),
                pm_hdl("reboot", ti<virt::enums::domain::ShutdownFlag>, PM_LIFT(dom.reboot), 213, "Domain is being rebooted",
                       PM_PREREQ(if (dom_state != virt::enums::domain::State::RUNNING) return error(201);)),
                pm_hdl("reset", no_flags, PM_LIFT(dom.reset), 214, "Domain was reset", PM_PREREQ(if (!dom.isActive()) return error(210);)),
                pm_hdl("suspend", no_flags, PM_LIFT(dom.suspend), 215, "Domain suspended",
                       PM_PREREQ(if (dom_state != virt::enums::domain::State::RUNNING) return error(201);)),
                pm_hdl("resume", no_flags, PM_LIFT(dom.resume), 212, "Domain resumed",
                       PM_PREREQ(if (dom_state != virt::enums::domain::State::PAUSED) return error(211);)),
                [&]() { return error(300); });
        },
        +[](const boost::json::value& val, JsonRes& json_res, virt::Domain& dom) -> DependsOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), DependsOutcome::FAILURE; };
            if (!val.is_string())
                return error(0);
            if (!dom.rename(val.get_string().c_str()))
                return error(205);
            return DependsOutcome::SUCCESS;
        },
        +[](const boost::json::value& val, JsonRes& json_res, virt::Domain& dom) -> DependsOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), DependsOutcome::FAILURE; };
            if (!val.is_int64())
                return error(0);
            if (!dom.setMemory(val.get_uint64()))
                return error(206);
            return DependsOutcome::SUCCESS;
        },
        +[](const boost::json::value& val, JsonRes& json_res, virt::Domain& dom) -> DependsOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), DependsOutcome::FAILURE; };
            if (!val.is_uint64())
                return error(0);
            if (!dom.setMaxMemory(val.get_uint64()))
                return error(207);
            return DependsOutcome::SUCCESS;
        },
        +[](const boost::json::value& val, JsonRes& json_res, virt::Domain& dom) -> DependsOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), DependsOutcome::FAILURE; };
            if (!val.is_bool())
                return error(0);
            if (!dom.setAutoStart(val.get_bool()))
                return error(208);
            return DependsOutcome::SUCCESS;
        },
        +[](const boost::json::value& val, JsonRes& json_res, virt::Domain& dom) -> DependsOutcome {
            const auto res = wrap_fcn(
                val, json_res, [&](auto... args) { return dom.sendProcessSignal(args...); }, WArg<JTag::Int64>{"pid"},
                WArg<JTag::Enum, JTag::None, virt::enums::domain::ProcessSignal>{"signal"});
            return res ? DependsOutcome::SUCCESS : DependsOutcome::FAILURE;
        },
        +[](const boost::json::value& val, JsonRes& json_res, virt::Domain& dom) -> DependsOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), DependsOutcome::FAILURE; };
            if (!val.is_object())
                return error(0);
            const auto& val_obj = val.get_object();

            const auto keycodeset_opt = extract_param<JTag::Enum, JTag::None, virt::enums::domain::KeycodeSet>(val_obj, "keycode_set", json_res);
            if (!keycodeset_opt)
                return error(0);
            const auto keycodeset = *keycodeset_opt;

            const auto holdtime_opt = extract_param<JTag::Uint64>(val_obj, "hold_time", json_res);
            if (!holdtime_opt)
                return error(0);
            const auto holdtime = *holdtime_opt;

            const auto keys_opt = extract_param<JTag::Array, JTag::Uint64>(val_obj, "keys", json_res);
            if (!keys_opt)
                return error(0);
            const auto keys = *keys_opt;
            std::vector<unsigned> narrow_keys{keys.begin(), keys.end()};

            return dom.sendKey(keycodeset, holdtime, narrow_keys) ? DependsOutcome::SUCCESS : DependsOutcome::FAILURE;
        }};
    static_assert(keys.size() == fcns.size());
} constexpr static const domain_actions_table{};
