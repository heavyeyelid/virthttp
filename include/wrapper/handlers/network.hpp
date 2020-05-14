#pragma once
#include <tuple>
#include <rapidjson/rapidjson.h>
#include "wrapper/depends.hpp"
#include "wrapper/dispatch.hpp"
#include "wrapper/network_actions_table.hpp"
#include "base.hpp"
#include "flagwork.hpp"
#include "hdl_ctx.hpp"
#include "logger.hpp"
#include "urlparser.hpp"
#include "virt_wrap.hpp"

/**
 * \internal
 * jdispatcher values for network handlers as a type list
 **/
using NetworkJDispatcherVals = std::tuple<JDispatchVals<JAll>, JDispatchVals<JAll>, JDispatchVals<JAll>, JDispatchVals<JAll>>;
/**
 * \internal
 * jdispatcher values for network handlers as a tuple
 **/
constexpr NetworkJDispatcherVals network_jdispatcher_vals{};

/**
 * \internal jdispatchers for network handlers as an array
 **/
constexpr auto network_jdispatchers = gen_jdispatchers(network_jdispatcher_vals);

/**
 * \internal
 * Networks-specific handler utilities
 **/
class NetworkUnawareHandlers : public HandlerContext {
    /**
     * Equivalent to calling #json_res's JsonRes::error
     * \tparam Args (deduced)
     * \param[in] args arguments to JsonRes::error
     **/
    template <class... Args> auto error(Args... args) const noexcept { return json_res.error(args...); };

  public:
    explicit NetworkUnawareHandlers(HandlerContext& ctx) : HandlerContext(ctx) {}

    /**
     * \internal
     * Extractor of virt::Connection::List::Networks::Flag from a URI's target
     *
     * \param[in] target the target to extract the flag from
     * \return the flag, or `std::nullopt` on error
     * */
    [[nodiscard]] constexpr auto search_all_flags(const TargetParser& target) const noexcept
        -> std::optional<virt::enums::connection::list::networks::Flag> {
        using namespace virt::enums::connection::list::networks;
        auto flags = Flag::DEFAULT;
        if (auto activity = target.getBool("active"); activity)
            flags |= *activity ? Flag::ACTIVE : Flag::INACTIVE;
        if (auto persistence = target.getBool("persistent"); persistence)
            flags |= *persistence ? Flag::PERSISTENT : Flag::TRANSIENT;
        if (auto autostart = target.getBool("autostart"); autostart)
            flags |= *autostart ? Flag::AUTOSTART : Flag::NO_AUTOSTART;
        return {flags};
    }
};

/**
 * \internal
 * Network-specific handlers
 **/
class NetworkHandlers : public HandlerMethods {
    template <class... Args> auto error(Args... args) const noexcept { return json_res.error(args...); };
    virt::Network& nw; ///< Current libvirt network

  public:
    /**
     * \internal
     **/
    explicit NetworkHandlers(HandlerContext& ctx, virt::Network& nw) : HandlerMethods(ctx), nw(nw) {}

    DependsOutcome create(const rapidjson::Value& obj) override { return error(-1), DependsOutcome::FAILURE; }

    DependsOutcome query(const rapidjson::Value& action) override {
        rapidjson::Value res_val;

        const auto outcome =
            parameterized_depends_scope(subquery("dhcp-leases", SUBQ_LIFT(nw.extractDHCPLeases), fwd_as_if_err(-2), [&](auto& jalloc) {
                rapidjson::Value json_dhcp_leases_array;
                json_dhcp_leases_array.SetArray();

                auto dhcp_leases = nw.extractDHCPLeases("Test");
                if (!dhcp_leases) {
                    logger.error("Error occured while getting DHCP Leases");
                    return error(-999), DependsOutcome::FAILURE;
                }
                for (auto& lease : *dhcp_leases) {
                    rapidjson::Value json_lease;
                    json_lease.AddMember("type", lease.type, json_res.GetAllocator());
                    json_lease.AddMember("client-id", to_json(lease.clientid, json_res.GetAllocator()), json_res.GetAllocator());
                    json_lease.AddMember("expiry-time", lease.expirytime, json_res.GetAllocator());
                    json_lease.AddMember("hostname", to_json(lease.hostname, json_res.GetAllocator()), json_res.GetAllocator());
                    json_lease.AddMember("iaid", to_json(lease.iaid, json_res.GetAllocator()), json_res.GetAllocator());
                    json_lease.AddMember("interface", to_json(lease.iface, json_res.GetAllocator()), json_res.GetAllocator());
                    json_lease.AddMember("ip-address", to_json(lease.ipaddr, json_res.GetAllocator()), json_res.GetAllocator());
                    json_lease.AddMember("mac-address", to_json(lease.mac, json_res.GetAllocator()), json_res.GetAllocator());
                    json_lease.AddMember("prefix", lease.prefix, json_res.GetAllocator());
                    json_dhcp_leases_array.PushBack(json_lease, json_res.GetAllocator());
                }
                return json_dhcp_leases_array;
            }))();
        //            subquery("dumpxml"), subquery("event"),
        //            subquery("info"), subquery("name"), subquery("uuid"), subquery("port-list"), subquery("port-dumpxml"));

        rapidjson::Value jsonActive;
        {
            TFE nwActive = nw.isActive();
            if (nwActive.err()) {
                logger.error("Error occurred while getting network status");
                return error(500), DependsOutcome::FAILURE;
            }
            jsonActive.SetBool(static_cast<bool>(nwActive));
        }
        {
            TFE nwAS = nw.isActive();
            if (nwAS.err()) {
                logger.error("Error occurred while getting network autostart policy");
                return error(500), DependsOutcome::FAILURE;
            }
            jsonActive.SetBool(static_cast<bool>(nwAS));
        }

        res_val.SetObject();
        res_val.AddMember("name", rapidjson::Value(nw.getName(), json_res.GetAllocator()), json_res.GetAllocator());
        res_val.AddMember("uuid", nw.extractUUIDString(), json_res.GetAllocator());
        res_val.AddMember("active", jsonActive, json_res.GetAllocator());
        json_res.result(std::move(res_val));
        return DependsOutcome::SUCCESS;
    }
    DependsOutcome alter(const rapidjson::Value& action) override {
        const auto& action_obj = *action.MemberBegin();
        const auto& [action_name, action_val] = action_obj;
        const auto hdl = network_actions_table[std::string_view{action_name.GetString(), action_name.GetStringLength()}];
        return hdl ? hdl(action_val, json_res, nw) : (error(123), DependsOutcome::FAILURE);
    }
    DependsOutcome vacuum(const rapidjson::Value& action) override { return error(-1), DependsOutcome::FAILURE; }
};