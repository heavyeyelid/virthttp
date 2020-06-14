#pragma once
#include <array>
#include <functional>
#include <string_view>
#include <gsl/gsl>
#include "virt_wrap/Domain.hpp"
#include "cexpr_algs.hpp"
#include "depends.hpp"
#include "json2virt.hpp"
#include "json_utils.hpp"
#include "utils.hpp"

#define PM_LIFT(mem_fn) [&](auto... args) { return mem_fn(args...); }
#define PM_PREREQ(...) [&] { __VA_ARGS__ return DependsOutcome::SUCCESS; }

using namespace std::literals;

constexpr auto action_scope = [](auto&&... actions) {
    using Arr = std::array<std::function<DependsOutcome()>, sizeof...(actions)>; // pray for SFO ; wait for expansion statements
    for (auto&& action : Arr{actions...}) {
        if (const auto ao = action(); ao != DependsOutcome::SKIPPED)
            return ao;
    }
    return DependsOutcome::SKIPPED;
};

template <class CRTP, class Hdl> class NamedCallTable {
    constexpr auto& keys() const noexcept { return static_cast<const CRTP&>(*this).keys; }
    constexpr auto& fcns() const noexcept { return static_cast<const CRTP&>(*this).fcns; }

  public:
    constexpr Hdl operator[](std::string_view sv) const noexcept {
        const auto it = cexpr::find(keys().begin(), keys().end(), sv);
        if (it == keys().end())
            return nullptr;
        const auto idx = std::distance(keys().begin(), it);
        return fcns()[idx];
    }
};

using DomainActionsHdl = DependsOutcome (*)(const boost::json::value& val, JsonRes& json_res, virt::Domain& dom);
class DomainActionsTable : public NamedCallTable<DomainActionsTable, DomainActionsHdl> {
  private:
    friend NamedCallTable<DomainActionsTable, DomainActionsHdl>;
    template <typename Flag>
    constexpr static const auto getFlag = [](const boost::json::string& json_flag, auto error) {
        if (auto v = Flag::from_string(json_flag); v)
            return std::optional{*v};
        return error(301), std::optional<Flag>{std::nullopt};
    };
    template <class F> static std::optional<F> getCombinedFlags(const boost::json::value& json_flag, JsonRes& json_res) noexcept {
        auto error = [&](auto... args) { return json_res.error(args...), std::nullopt; };
        auto getFlag = DomainActionsTable::getFlag<F>;

        F flagset{};
        if (json_flag.is_array()) {
            const auto json_arr = json_flag.get_array();
            if constexpr (test_sfinae([](auto f) -> decltype(f | f) { UNREACHABLE; }, F{})) {
                for (const auto& json_el : json_arr) {
                    if (!json_el.is_string())
                        return std::nullopt;
                    const auto v = getFlag(json_el.get_string(), error);
                    if (!v)
                        return std::nullopt;
                    flagset |= *v;
                }
            } else {
                if (json_arr.size() > 1)
                    return error(301);
                return {json_arr.empty() ? F{} : getFlag(json_arr[0], error)};
            }
        } else if (json_flag.is_string()) {
            const auto v = getFlag(json_flag.get_string(), error);
            if (!v)
                return std::nullopt;
            flagset = *v;
        }
        return {flagset};
    }

    using Hdl = DomainActionsHdl;

    constexpr static std::array<std::string_view, 7> keys = {"power_mgt", "name", "memory", "max_memory", "autostart", "send_signal", "send_keys"};
    constexpr static std::array<Hdl, 7> fcns = {
        +[](const boost::json::value& val, JsonRes& json_res, virt::Domain& dom) -> DependsOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), DependsOutcome::FAILURE; };
            auto pm_message = [&](gsl::czstring<> name, gsl::czstring<> value) {
                boost::json::object msg_val{};
                msg_val.emplace(name, value);
                json_res.message(msg_val);
                return DependsOutcome::SUCCESS;
            };

            constexpr auto getShutdownFlag = getFlag<virt::Domain::ShutdownFlag>;

            const boost::json::value* json_flag = nullptr;
            std::string_view pm_req;

            if (val.is_string()) {
                pm_req = val.get_string();
            } else if (val.is_object()) {
                auto& val_obj = val.get_object();
                auto it_req = val_obj.find("request");
                auto it_flags = val_obj.find("type");
                if (it_req == val_obj.end())
                    return error(300);
                if (it_flags != val_obj.end())
                    json_flag = &it_flags->value();
                pm_req = it_req->value().get_string();
            } else {
                return error(300);
            }

            const auto dom_state = virt::Domain::State{EHTag{}, dom.getInfo().state}; // Verified use of EHTag

            const auto pm_hdl = [&](gsl::czstring<> req_tag, auto flag_ti, auto mem_fcn, int errc, gsl::czstring<> pm_msg, auto prereqs) {
                using Flag = typename decltype(flag_ti)::type;
                return [=, &json_flag, &json_res]() {
                    const auto mem_fcn_passthru = [=](auto... args) constexpr { return mem_fcn(args...); }; // WA GCC
                    const auto local_error = [&] {
                        logger.error(error_messages[errc]);
                        return error(errc);
                    };

                    if (pm_req == req_tag) {
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
                pm_hdl("shutdown", ti<virt::Domain::ShutdownFlag>, PM_LIFT(dom.shutdown), 200, "Domain is being shutdown",
                       PM_PREREQ(if (dom_state != virt::Domain::State::RUNNING) return error(201);)),
                pm_hdl("destroy", ti<virt::Domain::DestroyFlag>, PM_LIFT(dom.destroy), 209, "Domain destroyed",
                       PM_PREREQ(if (!dom.isActive()) return error(210);)),
                pm_hdl("start", ti<virt::Domain::CreateFlag>, PM_LIFT(dom.create), 202, "Domain started",
                       PM_PREREQ(if (dom.isActive()) return error(203);)),
                pm_hdl("reboot", ti<virt::Domain::ShutdownFlag>, PM_LIFT(dom.reboot), 213, "Domain is being rebooted",
                       PM_PREREQ(if (dom_state != virt::Domain::State::RUNNING) return error(201);)),
                pm_hdl("reset", no_flags, PM_LIFT(dom.reset), 214, "Domain was reset", PM_PREREQ(if (!dom.isActive()) return error(210);)),
                pm_hdl("suspend", no_flags, PM_LIFT(dom.suspend), 215, "Domain suspended",
                       PM_PREREQ(if (dom_state != virt::Domain::State::RUNNING) return error(201);)),
                pm_hdl("resume", no_flags, PM_LIFT(dom.resume), 212, "Domain resumed",
                       PM_PREREQ(if (dom_state != virt::Domain::State::PAUSED) return error(211);)),
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
            boost::json::error_code ec;
            const auto memory_amount = boost::json::number_cast<std::uint64_t>(val);
            if (ec)
                return error(0);
            if (!dom.setMemory(memory_amount))
                return error(206);
            return DependsOutcome::SUCCESS;
        },
        +[](const boost::json::value& val, JsonRes& json_res, virt::Domain& dom) -> DependsOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), DependsOutcome::FAILURE; };
            boost::json::error_code ec;
            const auto memory_amount = boost::json::number_cast<std::uint64_t>(val);
            if (ec)
                return error(0);
            if (!dom.setMaxMemory(memory_amount))
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
                WArg<JTag::Enum, JTag::None, virt::Domain::ProcessSignal>{"signal"});
            return res ? DependsOutcome::SUCCESS : DependsOutcome::FAILURE;
        },
        +[](const boost::json::value& val, JsonRes& json_res, virt::Domain& dom) -> DependsOutcome {
            auto error = [&](auto... args) { return json_res.error(args...), DependsOutcome::FAILURE; };
            if (!val.is_object())
                return error(0);
            const auto& val_obj = val.get_object();
            const auto keycodeset_opt = extract_param<JTag::Enum, JTag::None, virt::Domain::KeycodeSet>(val_obj, "keycode_set", json_res);
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