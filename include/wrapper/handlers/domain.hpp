#pragma once
#include <tuple>
#include <rapidjson/rapidjson.h>
#include <virt_wrap/Error.hpp>
#include "wrapper/actions_table.hpp"
#include "wrapper/depends.hpp"
#include "wrapper/dispatch.hpp"
#include "wrapper/virt2json.hpp"
#include "base.hpp"
#include "flagwork.hpp"
#include "hdl_ctx.hpp"
#include "urlparser.hpp"
#include "virt_wrap.hpp"

using DomainJDispatcherVals =
    std::tuple<JDispatchVals<JTypeList<rapidjson::kStringType, rapidjson::kObjectType>, JTypeList<rapidjson::kArrayType>>, JDispatchVals<JAll>,
               JDispatchVals<JTypeList<rapidjson::kObjectType>, JTypeList<rapidjson::kArrayType>>, JDispatchVals<JAll>>;
constexpr DomainJDispatcherVals domain_jdispatcher_vals{};

constexpr std::array<JDispatch, std::tuple_size_v<DomainJDispatcherVals>> domain_jdispatchers = gen_jdispatchers(domain_jdispatcher_vals);

struct DomainUnawareHandlers : public HandlerContext {
    explicit DomainUnawareHandlers(HandlerContext& ctx) : HandlerContext(ctx) {}

    [[nodiscard]] constexpr auto search_all_flags(const TargetParser& target) const noexcept -> std::optional<virt::Connection::List::Domains::Flag> {
        auto flags = virt::Connection::List::Domains::Flag::DEFAULT;
        if (auto activity = target.getBool("active"); activity)
            flags |= *activity ? virt::Connection::List::Domains::Flag::ACTIVE : virt::Connection::List::Domains::Flag::INACTIVE;
        if (auto persistence = target.getBool("persistent"); persistence)
            flags |= *persistence ? virt::Connection::List::Domains::Flag::PERSISTENT : virt::Connection::List::Domains::Flag::TRANSIENT;
        if (auto savemgmt = target.getBool("managed_save"); savemgmt)
            flags |= *savemgmt ? virt::Connection::List::Domains::Flag::MANAGEDSAVE : virt::Connection::List::Domains::Flag::NO_MANAGEDSAVE;
        if (auto autostart = target.getBool("autostart"); autostart)
            flags |= *autostart ? virt::Connection::List::Domains::Flag::AUTOSTART : virt::Connection::List::Domains::Flag::NO_AUTOSTART;
        if (auto snapshot = target.getBool("has_snapshot"); snapshot)
            flags |= *snapshot ? virt::Connection::List::Domains::Flag::HAS_SNAPSHOT : virt::Connection::List::Domains::Flag::NO_SNAPSHOT;

        const auto opt_flags =
            target_get_composable_flag<virt::Connection::List::Domains::Flag, virt::Connection::List::Domains::FlagsC>(target, "status");
        if (!opt_flags)
            return error(301), std::nullopt;
        return {flags | *opt_flags};
    }
};

class DomainHandlers : public HandlerMethods {
    virt::Domain& dom;

  public:
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
        auto& path_parts = target.getPathParts();
        if (path_parts.size() < 5) {
            res_val.SetObject();
            const auto [state, max_mem, memory, nvirt_cpu, cpu_time] = dom.getInfo();
            const auto os_type = dom.getOSType();
            res_val.AddMember("name", rapidjson::Value(dom.getName(), json_res.GetAllocator()), json_res.GetAllocator());
            res_val.AddMember("uuid", dom.extractUUIDString(), json_res.GetAllocator());
            res_val.AddMember("id", static_cast<int>(dom.getID()), json_res.GetAllocator());
            res_val.AddMember("status", rapidjson::StringRef(virt::Domain::States[state]), json_res.GetAllocator());
            res_val.AddMember("os", rapidjson::Value(os_type.get(), json_res.GetAllocator()), json_res.GetAllocator());
            res_val.AddMember("ram", memory, json_res.GetAllocator());
            res_val.AddMember("ram_max", max_mem, json_res.GetAllocator());
            res_val.AddMember("cpu", nvirt_cpu, json_res.GetAllocator());
        } else if (path_parts.size() == 5) {
            if (path_parts[4] == "xml_desc") {
                const auto opt_flags = target_get_composable_flag<virt::Domain::XmlFlag, virt::Domain::XmlFlagsC>(target, "options");
                if (!opt_flags)
                    return error(301), DependsOutcome::FAILURE;
                res_val = rapidjson::Value(dom.getXMLDesc(*opt_flags), json_res.GetAllocator());
            }
        }
        json_res.result(std::move(res_val));

        auto res = parameterized_depends_scope(
            subquery(
                "xml_desc", "options", SUBQ_LIFT(dom.getXMLDesc),
                [&](auto xml) {
                    return xml ? std::pair{rapidjson::Value(xml, json_res.GetAllocator()), true}
                               : (error(-999), std::pair{rapidjson::Value{}, false});
                },
                tp<virt::Domain::XmlFlag, virt::Domain::XmlFlagsC>),
            subquery("fs_info", SUBQ_LIFT(dom.getFSInfo),
                     [&](auto fs_infos) {
                         if (!fs_infos.get()) {
                             error(-999); // getting filesystem information failed
                             return std::pair{rapidjson::Value{}, false};
                         }
                         rapidjson::Value jvres;
                         for (const virDomainFSInfo& fs_info : fs_infos) {
                             rapidjson::Value sub;
                             sub.AddMember("name", rapidjson::Value(fs_info.name, json_res.GetAllocator()), json_res.GetAllocator());
                             sub.AddMember("mountpoint", rapidjson::Value(fs_info.mountpoint, json_res.GetAllocator()), json_res.GetAllocator());
                             sub.AddMember("fs_type", rapidjson::Value(fs_info.fstype, json_res.GetAllocator()), json_res.GetAllocator());
                             {
                                 rapidjson::Value dev_aliases;
                                 dev_aliases.SetArray();
                                 for (const char* dev_alias : gsl::span{fs_info.devAlias, static_cast<long>(fs_info.ndevAlias)})
                                     dev_aliases.PushBack(rapidjson::Value(dev_alias, json_res.GetAllocator()), json_res.GetAllocator());
                                 sub.AddMember("disk_dev_aliases", dev_aliases, json_res.GetAllocator());
                             }
                             jvres.PushBack(sub, json_res.GetAllocator());
                         }
                         return std::pair{std::move(jvres), true};
                     }),
            subquery("hostname", SUBQ_LIFT(dom.getHostname),
                     [&](std::optional<UniqueZstring> hostname) {
                         return hostname ? std::pair{rapidjson::Value(static_cast<const char*>(*hostname), json_res.GetAllocator()), true}
                                         : std::pair{rapidjson::Value{}, false};
                     }),
            subquery("launch_security_info", SUBQ_LIFT(dom.getLaunchSecurityInfo), [&](const auto& otp) {
                return otp ? std::pair{to_json(*otp, json_res.GetAllocator()), true} : (error(-999), std::pair{rapidjson::Value{}, false});
            }))(4, target, res_val, [&](auto... args) { return error(args...); });
        return DependsOutcome::SUCCESS;
    }
    DependsOutcome alter(const rapidjson::Value& action) override {
        const auto& action_obj = *action.MemberBegin();
        const auto& [action_name, action_val] = action_obj;
        const auto hdl = domain_actions_table[std::string_view{action_name.GetString(), action_name.GetStringLength()}];
        return hdl ? hdl(action_val, json_res, dom, key_str) : (error(123), DependsOutcome::FAILURE);
    }
    DependsOutcome vacuum(const rapidjson::Value& action) override {
        auto success = [&] {
            rapidjson::Value res_val;
            res_val.SetObject();
            res_val.AddMember("deleted", true, json_res.GetAllocator());
            json_res.result(std::move(res_val));
            return DependsOutcome::SUCCESS;
        };
        auto failure = [&] {
            rapidjson::Value msg_val;
            msg_val.SetObject();
            msg_val.AddMember("libvirt", rapidjson::Value(virt::extractLastError().message, json_res.GetAllocator()), json_res.GetAllocator());
            json_res.message(std::move(msg_val));
            return error(216), DependsOutcome::FAILURE;
        };
        const auto opt_flags = target_get_composable_flag<virt::Domain::UndefineFlag, virt::Domain::UndefineFlagsC>(target, "options");
        if (!opt_flags)
            return error(301), DependsOutcome::FAILURE;
        return dom.undefine(*opt_flags) ? success() : failure();
    }
};