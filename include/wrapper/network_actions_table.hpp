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

    constexpr static std::array<std::string_view, 7> keys = {"power_mgt", "name", "memory", "max_memory", "autostart", "send_signal", "send_keys"};
    constexpr static std::array<Hdl, 7> fcns = {};
    static_assert(keys.size() == fcns.size());
} constexpr static const network_actions_table{};
