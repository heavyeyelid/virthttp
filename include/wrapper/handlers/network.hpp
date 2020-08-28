#pragma once
#include <tuple>
#include <boost/json.hpp>
#include "wrapper/actions_table.hpp"
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
    [[nodiscard]] static constexpr auto search_all_flags(const TargetParser& target) noexcept
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

    auto create(const boost::json::value& obj) -> DependsOutcome override {
        const auto create_nw = [&](std::string_view xml) {
            nw = virt::Network::createXML(conn, xml.data());
            if (!nw)
                return error(-999), DependsOutcome::FAILURE;
            boost::json::value res_val;
            json_res.result(boost::json::object{{"created", true}});
            return DependsOutcome::SUCCESS;
        };

        if (obj.is_string())
            return create_nw(obj.get_string());
        if (obj.is_array()) {
            for (const auto& item : obj.get_array()) {
                if (!item.is_string())
                    return error(-999), DependsOutcome::FAILURE; // Not a string array
                if (create_nw(item.get_string()) == DependsOutcome::FAILURE)
                    return DependsOutcome::FAILURE; // Error while creating network
            }
            return DependsOutcome::SUCCESS;
        }
        return error(-1), DependsOutcome::FAILURE;
    }

    auto query(const boost::json::value& action) -> DependsOutcome override {
        boost::json::value res_val;
        const auto& path_parts = target.getPathParts();
        if (path_parts.size() < 5) {
            const TFE nw_active = nw.isActive();
            if (nw_active.err()) {
                logger.error("Error occurred while getting network status");
                return error(500), DependsOutcome::FAILURE;
            }
            const auto json_active = static_cast<bool>(nw_active);

            const TFE nw_autostart = nw.getAutostart();
            if (nw_autostart.err()) {
                logger.error("Error occurred while getting network autostart policy");
                return error(500), DependsOutcome::FAILURE;
            }
            const auto json_autostart = static_cast<bool>(nw_autostart);

            boost::json::value json_is_persistent;
            {
                const TFE tfe = nw.isPersistent();
                if (tfe.err()) {
                    logger.error("Error occurred while getting network persistence");
                    return error(500), DependsOutcome::FAILURE;
                }
                json_is_persistent = to_json(tfe);
            }

            boost::json::object res_val = {{"name", nw.getName()},
                                           {"uuid", nw.extractUUIDString()},
                                           {"active", json_active},
                                           {"autostart", json_autostart},
                                           {"persistent", json_is_persistent}};

            if (path_parts.size() == 4)
                res_val.emplace("bridge", to_json(nw.getBridgeName()));
            json_res.result(std::move(res_val));
            return DependsOutcome::SUCCESS;
        }

        const auto outcome =
            parameterized_depends_scope(subquery("dhcp-leases", "mac", ti<std::string>, SUBQ_LIFT(nw.extractDHCPLeases), fwd_as_if_err(-2)),
                                        subquery("dumpxml", "options", ti<virt::enums::network::XMLFlags>, SUBQ_LIFT(nw.getXMLDesc),
                                                 fwd_as_if_err(-2)))(4, target, res_val, [&](auto... args) { return error(args...); });

        if (outcome == DependsOutcome::SUCCESS)
            json_res.result(std::move(res_val));
        return outcome;
    }

    auto alter(const boost::json::value& action) -> DependsOutcome override {
        const auto& [key, value] = *action.get_object().begin();
        const auto hdl = network_actions_table[key];
        return hdl ? hdl(value, json_res, nw) : (error(123), DependsOutcome::FAILURE);
    }

    auto vacuum(const boost::json::value& action) -> DependsOutcome override {
        const auto success = [&] {
            json_res.result(boost::json::object{{"deleted", true}});
            return DependsOutcome::SUCCESS;
        };
        const auto failure = [&] {
            json_res.message(boost::json::object{{"libvirt", virt::extractLastError().message}});
            return error(-999), DependsOutcome::FAILURE;
        };
        return nw.undefine() ? success() : failure();
    }
};