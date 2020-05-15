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

using NetworkActionsHdl = DependsOutcome (*)(const rapidjson::Value& val, JsonRes& json_res, virt::Network& nw);
class NetworkActionsTable : public NamedCallTable<NetworkActionsTable, NetworkActionsHdl> {
  private:
    friend NamedCallTable<NetworkActionsTable, NetworkActionsHdl>;

    using Hdl = NetworkActionsHdl;

    constexpr static std::array<std::string_view, 2> keys = {"autostart", "action"};
    constexpr static std::array<Hdl, 2> fcns = {+[](const rapidjson::Value& val, JsonRes& json_res, virt::Network& nw) -> DependsOutcome {
                                                    if (!val.IsBool())
                                                        return DependsOutcome::FAILURE;
                                                    if (!nw.setAutostart(val.GetBool()))
                                                        return DependsOutcome::FAILURE;
                                                    return DependsOutcome::SUCCESS;
                                                },
                                                +[](const rapidjson::Value& val, JsonRes& json_res, virt::Network& nw) -> DependsOutcome {
                                                    const auto error = [&](const auto... args) {
                                                        return json_res.error(args...), DependsOutcome::FAILURE;
                                                    };
                                                    std::string_view action;
                                                    if (val.IsString()) {
                                                        action = val.GetString();
                                                    }
                                                    if (action == "start") {
                                                        if (!nw.create())
                                                            return error(-999);
                                                    }
                                                    if (action == "stop") {
                                                        if (!nw.destroy())
                                                            return error(-999);
                                                    }
                                                    return DependsOutcome::SUCCESS;
                                                }};
    static_assert(keys.size() == fcns.size());
} constexpr static const network_actions_table{};
