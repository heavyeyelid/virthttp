#pragma once
#include "wrapper/actions_table.hpp"
#include "wrapper/depends.hpp"
#include "base.hpp"
#include "virt_wrap.hpp"

class DomainHandlers : HandlerContext {

    template <class... Args> auto error(Args... args) { return json_res.error(args...); };

    virt::Domain& dom;

  public:
    explicit DomainHandlers(HandlerContext& ctx, virt::Domain& dom) : HandlerContext(ctx), dom(dom) {}

    DependsOutcome creation(const rapidjson::Value& obj, bool known_str = false, bool known_obj = false) {
        if (known_str || obj.IsString()) {
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

    DependsOutcome modification(const rapidjson::Value& action) {
        const auto& action_obj = *action.MemberBegin();
        const auto& [action_name, action_val] = action_obj;
        const auto hdl = domain_actions_table[std::string_view{action_name.GetString(), action_name.GetStringLength()}];
        return hdl ? hdl(action_val, json_res, dom, key_str) : (error(123), DependsOutcome::FAILURE);
    }
    DependsOutcome query(const rapidjson::Value& action) {
        rapidjson::Value res_val;
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

        json_res.result(std::move(res_val));
        return DependsOutcome::SUCCESS;
    }
};