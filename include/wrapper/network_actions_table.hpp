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

    constexpr static std::array<std::string_view, 1> keys = {"autostart"};
    constexpr static std::array<Hdl, 1> fcns = {};
    static_assert(keys.size() == fcns.size());
} constexpr static const network_actions_table{};
