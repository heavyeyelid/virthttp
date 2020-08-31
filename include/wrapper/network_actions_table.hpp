//
// Created by hugo on 4/5/20.
//

#pragma once

#include <array>
#include <functional>
#include <string_view>
#include <gsl/gsl>
#include "virt_wrap/Network.hpp"
#include "wrapper/actions_table.hpp"
#include "cexpr_algs.hpp"
#include "depends.hpp"
#include "json2virt.hpp"
#include "json_utils.hpp"
#include "utils.hpp"

using NetworkActionsHdl = DependsOutcome (*)(const boost::json::value& val, JsonRes& json_res, virt::Network& nw);
class NetworkActionsTable : public NamedCallTable<NetworkActionsTable, NetworkActionsHdl> {
  private:
    friend NamedCallTable<NetworkActionsTable, NetworkActionsHdl>;

    using Hdl = NetworkActionsHdl;

    constexpr static std::array<std::string_view, 2> keys = {"autostart", "action"};
    constexpr static std::array<Hdl, 2> fcns = {+[](const boost::json::value& val, JsonRes& json_res, virt::Network& nw) -> DependsOutcome {
                                                    auto error = [&](auto... args) { return json_res.error(args...), DependsOutcome::FAILURE; };

                                                    if (!val.is_bool())
                                                        return error(0);
                                                    if (!nw.setAutostart(val.get_bool()))
                                                        return error(999);
                                                    return DependsOutcome::SUCCESS;
                                                },
                                                +[](const boost::json::value& val, JsonRes& json_res, virt::Network& nw) -> DependsOutcome {
                                                    auto error = [&](auto... args) { return json_res.error(args...), DependsOutcome::FAILURE; };
                                                    auto message = [&](gsl::czstring<> name, gsl::czstring<> value) {
                                                        json_res.message(boost::json::object{{name, value}});
                                                        return DependsOutcome::SUCCESS;
                                                    };
                                                    if (!val.is_string())
                                                        return error(0);
                                                    const auto action_string = val.get_string();
                                                    if (action_string == "start") {
                                                        if (nw.create())
                                                            return message(action_string.c_str(), "Network started");
                                                        return error(999);
                                                    }

                                                    else if (action_string == "destroy") {
                                                        if (nw.destroy())
                                                            return message(action_string.c_str(), "Network destroyed");
                                                        return error(999);
                                                    }
                                                    return error(0);
                                                }};
    static_assert(keys.size() == fcns.size());
} constexpr static const network_actions_table{};
