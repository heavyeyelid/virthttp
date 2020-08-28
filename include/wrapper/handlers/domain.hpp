#pragma once
#include <optional>
#include <tuple>
#include <boost/json.hpp>
#include <virt_wrap/Error.hpp>
#include "wrapper/depends.hpp"
#include "wrapper/dispatch.hpp"
#include "wrapper/domain_actions_table.hpp"
#include "wrapper/virt2json.hpp"
#include "base.hpp"
#include "flagwork.hpp"
#include "hdl_ctx.hpp"
#include "urlparser.hpp"
#include "virt_wrap.hpp"

/**
 * \internal
 * jdispatcher values for domain handlers as a type list
 **/
using DomainJDispatcherVals =
    std::tuple<JDispatchVals<JTypeList<boost::json::kind::string, boost::json::kind::object>, JTypeList<boost::json::kind::array>>,
               JDispatchVals<JAll>, JDispatchVals<JTypeList<boost::json::kind::object>, JTypeList<boost::json::kind::array>>, JDispatchVals<JAll>>;
/**
 * \internal
 * jdispatcher values for domain handlers as a tuple
 **/
constexpr DomainJDispatcherVals domain_jdispatcher_vals{};

/**
 * \internal jdispatchers for domain handlers as an array
 **/
constexpr std::array<JDispatch, std::tuple_size_v<DomainJDispatcherVals>> domain_jdispatchers = gen_jdispatchers(domain_jdispatcher_vals);

/**
 * \internal
 * Domains-specific handler utilities
 **/
struct DomainUnawareHandlers : public HandlerContext {
    explicit DomainUnawareHandlers(HandlerContext& ctx) : HandlerContext(ctx) {}

    /**
     * \internal
     * Extractor of virt::Connection::List::Domains::Flag from a URI's target
     *
     * \param[in] target the target to extract the flag from
     * \return the flag, or `std::nullopt` on error
     * */
    [[nodiscard]] constexpr auto search_all_flags(const TargetParser& target) const noexcept
        -> std::optional<virt::enums::connection::list::domains::Flag> {
        using namespace virt::enums::connection::list::domains;
        Flag flags = Flag::DEFAULT;
        if (auto activity = target.getBool("active"); activity)
            flags |= *activity ? Flag::ACTIVE : Flag::INACTIVE;
        if (auto persistence = target.getBool("persistent"); persistence)
            flags |= *persistence ? Flag::PERSISTENT : Flag::TRANSIENT;
        if (auto savemgmt = target.getBool("managed_save"); savemgmt)
            flags |= *savemgmt ? Flag::MANAGEDSAVE : Flag::NO_MANAGEDSAVE;
        if (auto autostart = target.getBool("autostart"); autostart)
            flags |= *autostart ? Flag::AUTOSTART : Flag::NO_AUTOSTART;
        if (auto snapshot = target.getBool("has_snapshot"); snapshot)
            flags |= *snapshot ? Flag::HAS_SNAPSHOT : Flag::NO_SNAPSHOT;

        const auto opt_flags = target_get_composable_flag<Flag>(target, "state");
        if (!opt_flags)
            return error(301), std::nullopt;
        return {flags | *opt_flags};
    }
};

/**
 * \internal
 * Domain-specific handlers
 **/
class DomainHandlers : public HandlerMethods {
    virt::Domain& dom; ///< Current libvirt domain

  public:
    /**
     * \internal
     **/
    explicit DomainHandlers(HandlerContext& ctx, virt::Domain& dom) : HandlerMethods(ctx), dom(dom) {}

    DependsOutcome create(const boost::json::value& obj) override {
        if (obj.is_string()) {
            const auto& jstr = obj.get_string();
            dom = virt::Domain::createXML(conn, jstr.c_str());
            if (!dom)
                return error(105), DependsOutcome::FAILURE;
            boost::json::object res_val;
            res_val.emplace("created", true);
            json_res.result(std::move(res_val));
            return DependsOutcome::SUCCESS;
        }
        if (obj.is_object())
            return error(-1), DependsOutcome::FAILURE;
        return error(0), DependsOutcome::FAILURE;
    }

    DependsOutcome query(const boost::json::value& action) override {
        boost::json::value res_val;
        const auto& path_parts = target.getPathParts();
        if (path_parts.size() < 5) {
            auto& res_obj = res_val.emplace_object();
            const auto [state, max_mem, memory, nvirt_cpu, cpu_time] = dom.getInfo();
            const auto os_type = dom.getOSType();
            res_obj.emplace("name", dom.getName());
            res_obj.emplace("uuid", dom.extractUUIDString());
            res_obj.emplace("id", static_cast<int>(dom.getID()));
            res_obj.emplace("status", virt::enums::domain::State(EHTag{}, state).to_string());
            res_obj.emplace("os", os_type.get());
            res_obj.emplace("ram", memory);
            res_obj.emplace("ram_max", max_mem);
            res_obj.emplace("cpu", nvirt_cpu);
            json_res.result(std::move(res_val));
            return DependsOutcome::SUCCESS;
        }

        const auto fwd_err = [&](bool fwd, int code) -> bool {
            if (!fwd)
                error(code);
            return fwd;
        };
        const auto fwd_as_pred_err = [&](int code, auto pred) { return [&, code, pred](const auto& arg) { return fwd_err(pred(arg), code); }; };
        const auto fwd_as_if_err = [&](int code) { return [&, code](const auto& arg) { return fwd_err(static_cast<bool>(arg), code); }; };

        const auto outcome = parameterized_depends_scope(
            subquery("xml_desc", "options", ti<virt::enums::domain::XMLFlags>, SUBQ_LIFT(dom.getXMLDesc), fwd_as_if_err(-2)),
            subquery("fs_info", SUBQ_LIFT(dom.getFSInfo), fwd_as_if_err(201), // getting filesystem information failed
                     [&](auto fs_infos) {
                         boost::json::array jvres;
                         for (const virDomainFSInfo& fs_info : fs_infos) {
                             auto& sub = jvres.emplace_back(boost::json::object_kind).get_object();
                             sub.emplace("name", fs_info.name);
                             sub.emplace("mountpoint", fs_info.mountpoint);
                             sub.emplace("fs_type", fs_info.fstype);
                             auto& dev_aliases = sub["disk_dev_aliases"].emplace_array();
                             for (const char* dev_alias : gsl::span{fs_info.devAlias, static_cast<unsigned>(fs_info.ndevAlias)})
                                 dev_aliases.emplace_back(dev_alias);
                         }
                         return jvres;
                     }),
            subquery("hostname", SUBQ_LIFT(dom.getHostname), fwd_as_if_err(-2)), subquery("time", SUBQ_LIFT(dom.getTime), fwd_as_if_err(-2)),
            subquery("max_memory", SUBQ_LIFT(dom.getMaxMemory), fwd_as_if_err(-2)),
            subquery("max_vcpus", SUBQ_LIFT(dom.getMaxVcpus), fwd_as_pred_err(-2, [](int val) { return val >= 0; })),
            subquery("num_vcpus", "options", ti<virt::enums::domain::VCpuFlag>, SUBQ_LIFT(dom.getNumVcpus),
                     fwd_as_pred_err(-2, [](int val) { return val >= 0; })),
            subquery(
                "scheduler_type", SUBQ_LIFT(dom.getSchedulerType), [&](const auto& sp) { return fwd_err(sp.second, -2); },
                [&](auto sp) -> boost::json::object {
                    return {{"type", static_cast<const char*>(sp.first)}, {"params_count", static_cast<int>(sp.second)}};
                }),
            subquery("launch_security_info", SUBQ_LIFT(dom.getLaunchSecurityInfo), fwd_as_if_err(-2)))(4, target, res_val,
                                                                                                       [&](auto... args) { return error(args...); });
        if (outcome == DependsOutcome::SUCCESS)
            json_res.result(std::move(res_val));
        return outcome;
    }
    DependsOutcome alter(const boost::json::value& action) override {
        const auto& [key, value] = *action.get_object().begin();
        const auto hdl = domain_actions_table[key];
        return hdl ? hdl(value, json_res, dom) : (error(123), DependsOutcome::FAILURE);
    }
    DependsOutcome vacuum(const boost::json::value& action) override {
        auto success = [&] {
            json_res.result(boost::json::object{{"deleted", true}});
            return DependsOutcome::SUCCESS;
        };
        auto failure = [&] {
            json_res.message(boost::json::object{{"libvirt", virt::extractLastError().message}});
            return error(216), DependsOutcome::FAILURE;
        };
        const auto opt_flags = target_get_composable_flag<virt::enums::domain::UndefineFlag>(target, "options");
        if (!opt_flags)
            return error(301), DependsOutcome::FAILURE;
        return dom.undefine(*opt_flags) ? success() : failure();
    }
};