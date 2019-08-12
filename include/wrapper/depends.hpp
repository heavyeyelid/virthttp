#pragma once
#include <vector>
#include <rapidjson/document.h>
#include "json_utils.hpp"
#include "utils.hpp"

enum class DependsOutcome { SUCCESS, FAILURE, SKIPPED };

bool check_depends(const rapidjson::Value& depends_json, std::vector<DependsOutcome>& outcomes, JsonRes& json_res) noexcept;

template <typename Hdl> void handle_depends(const rapidjson::Value& json_req, JsonRes& json_res, Hdl&& hdl) {
    auto error = [&](auto... args) { return json_res.error(args...); };

    if (!json_req.IsArray())
        return error(298);

    std::vector<DependsOutcome> outcomes{};
    outcomes.reserve(json_req.Size());

    for (const auto& action : json_req.GetArray()) {
        if (check_depends(action, outcomes, json_res))
            outcomes.push_back(hdl(action));
    }
}