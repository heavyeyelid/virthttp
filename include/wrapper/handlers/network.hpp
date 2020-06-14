#pragma once
#include <tuple>
#include <boost/json.hpp>
#include "wrapper/actions_table.hpp"
#include "wrapper/depends.hpp"
#include "wrapper/dispatch.hpp"
#include "base.hpp"
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
        -> std::optional<virt::Connection::List::Networks::Flag> {
        auto flags = virt::Connection::List::Networks::Flag::DEFAULT;
        if (auto activity = target.getBool("active"); activity)
            flags |= *activity ? virt::Connection::List::Networks::Flag::ACTIVE : virt::Connection::List::Networks::Flag::INACTIVE;
        if (auto persistence = target.getBool("persistent"); persistence)
            flags |= *persistence ? virt::Connection::List::Networks::Flag::PERSISTENT : virt::Connection::List::Networks::Flag::TRANSIENT;
        if (auto autostart = target.getBool("autostart"); autostart)
            flags |= *autostart ? virt::Connection::List::Networks::Flag::AUTOSTART : virt::Connection::List::Networks::Flag::NO_AUTOSTART;
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

    DependsOutcome create(const boost::json::value& obj) override { return error(-1), DependsOutcome::FAILURE; }

    DependsOutcome query(const boost::json::value& action) override {
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

        json_res.result(boost::json::object{
            {"name", nw.getName()},
            {"uuid", nw.extractUUIDString()},
            {"active", json_active},
            {"autostart", json_autostart},
        });
        return DependsOutcome::SUCCESS;
    }
    DependsOutcome alter(const boost::json::value& action) override { return error(-1), DependsOutcome::FAILURE; }
    DependsOutcome vacuum(const boost::json::value& action) override { return error(-1), DependsOutcome::FAILURE; }
};