#pragma once
#include <optional>
#include <tuple>
#include <rapidjson/rapidjson.h>
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
    std::tuple<JDispatchVals<JTypeList<rapidjson::kStringType, rapidjson::kObjectType>, JTypeList<rapidjson::kArrayType>>, JDispatchVals<JAll>,
               JDispatchVals<JTypeList<rapidjson::kObjectType>, JTypeList<rapidjson::kArrayType>>, JDispatchVals<JAll>>;
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

    DependsOutcome create(const rapidjson::Value& obj) override {
        if (obj.IsString()) {
            dom = virt::Domain::createXML(conn, obj.GetString());
            if (!dom)
                return error(105), DependsOutcome::FAILURE;
            rapidjson::Value res_val;
            res_val.SetObject();
            res_val.AddMember("created", true, json_res.GetAllocator());
            json_res.result(std::move(res_val));
            return DependsOutcome::SUCCESS;
        }
        if (obj.IsObject())
            return error(-1), DependsOutcome::FAILURE;
        return error(0), DependsOutcome::FAILURE;
    }

    DependsOutcome query(const rapidjson::Value& action) override {
        rapidjson::Value res_val;
        auto& jalloc = json_res.GetAllocator();
        auto& path_parts = target.getPathParts();
        if (path_parts.size() < 5) {
            res_val.SetObject();
            const auto [state, max_mem, memory, nvirt_cpu, cpu_time] = dom.getInfo();
            const auto os_type = dom.getOSType();
            res_val.AddMember("name", rapidjson::Value(dom.getName(), jalloc), jalloc);
            res_val.AddMember("uuid", dom.extractUUIDString(), jalloc);
            res_val.AddMember("id", static_cast<int>(dom.getID()), jalloc);
            res_val.AddMember("status", rapidjson::StringRef(virt::enums::domain::State(EHTag{}, state).to_string().data()), jalloc);
            res_val.AddMember("os", rapidjson::Value(os_type.get(), jalloc), jalloc);
            res_val.AddMember("ram", memory, jalloc);
            res_val.AddMember("ram_max", max_mem, jalloc);
            res_val.AddMember("cpu", nvirt_cpu, jalloc);
            json_res.result(std::move(res_val));
            return DependsOutcome::SUCCESS;
        }



        const auto outcome = parameterized_depends_scope(
            subquery("xml_desc", "options", ti<virt::enums::domain::XmlFlag>, SUBQ_LIFT(dom.getXMLDesc), fwd_as_if_err(-2)),
            subquery("fs_info", SUBQ_LIFT(dom.getFSInfo), fwd_as_if_err(201), // getting filesystem information failed
                     [&](auto fs_infos, auto& jalloc) {
                         rapidjson::Value jvres;
                         for (const virDomainFSInfo& fs_info : fs_infos) {
                             rapidjson::Value sub;
                             sub.AddMember("name", rapidjson::Value(fs_info.name, jalloc), jalloc);
                             sub.AddMember("mountpoint", rapidjson::Value(fs_info.mountpoint, jalloc), jalloc);
                             sub.AddMember("fs_type", rapidjson::Value(fs_info.fstype, jalloc), jalloc);
                             {
                                 rapidjson::Value dev_aliases;
                                 dev_aliases.SetArray();
                                 for (const char* dev_alias : gsl::span{fs_info.devAlias, static_cast<long>(fs_info.ndevAlias)})
                                     dev_aliases.PushBack(rapidjson::Value(dev_alias, jalloc), jalloc);
                                 sub.AddMember("disk_dev_aliases", dev_aliases, jalloc);
                             }
                             jvres.PushBack(sub, jalloc);
                         }
                         return jvres;
                     }),
            subquery("hostname", SUBQ_LIFT(dom.getHostname), fwd_as_if_err(-2)), subquery("time", SUBQ_LIFT(dom.getTime), fwd_as_if_err(-2)),
            subquery(
                "scheduler_type", SUBQ_LIFT(dom.getSchedulerType), [&](const auto& sp) { return fwd_err(sp.second, -2); },
                [&](auto sp, auto& jalloc) {
                    rapidjson::Value ret{};
                    ret.SetObject();
                    ret.AddMember("type", rapidjson::Value(static_cast<const char*>(sp.first), jalloc), jalloc);
                    ret.AddMember("params_count", static_cast<int>(sp.second), jalloc);
                    return ret;
                }),
            subquery("launch_security_info", SUBQ_LIFT(dom.getLaunchSecurityInfo), fwd_as_if_err(-2)))(4, target, res_val, json_res.GetAllocator(),
                                                                                                       [&](auto... args) { return error(args...); });
        if (outcome == DependsOutcome::SUCCESS)
            json_res.result(std::move(res_val));
        return outcome;
    }
    DependsOutcome alter(const rapidjson::Value& action) override {
        const auto& action_obj = *action.MemberBegin();
        const auto& [action_name, action_val] = action_obj;
        const auto hdl = domain_actions_table[std::string_view{action_name.GetString(), action_name.GetStringLength()}];
        return hdl ? hdl(action_val, json_res, dom) : (error(123), DependsOutcome::FAILURE);
    }
    DependsOutcome vacuum(const rapidjson::Value& action) override {
        auto& jalloc = json_res.GetAllocator();
        auto success = [&] {
            rapidjson::Value res_val;
            res_val.SetObject();
            res_val.AddMember("deleted", true, jalloc);
            json_res.result(std::move(res_val));
            return DependsOutcome::SUCCESS;
        };
        auto failure = [&] {
            rapidjson::Value msg_val;
            msg_val.SetObject();
            msg_val.AddMember("libvirt", rapidjson::Value(virt::extractLastError().message, jalloc), jalloc);
            json_res.message(std::move(msg_val));
            return error(216), DependsOutcome::FAILURE;
        };
        const auto opt_flags = target_get_composable_flag<virt::enums::domain::UndefineFlag>(target, "options");
        if (!opt_flags)
            return error(301), DependsOutcome::FAILURE;
        return dom.undefine(*opt_flags) ? success() : failure();
    }
};