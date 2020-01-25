#pragma once
#include <vector>
#include <rapidjson/document.h>
#include "json_utils.hpp"
#include "utils.hpp"

/**
 * \internal
 * Possible outcomes of a dependent action
 **/
enum class DependsOutcome { SUCCESS, FAILURE, SKIPPED };

/**
 * \internal
 * Checks the dependency status of an action
 *
 * \param[in] depends_json (possibly) dependent action
 * \param[in] outcomes the current stack of previously obtained outcomes
 * \param[in] json_res the response body, as JSON
 * \return `true` if the dependency chain is holding, `false` if broken
 **/
bool check_depends(const rapidjson::Value& depends_json, std::vector<DependsOutcome>& outcomes, JsonRes& json_res) noexcept;

/**
 * \internal
 * Calls an action handler on all actions which dependency chain is holding
 *
 * \tparam Hdl (deduced)
 * \param[in] json_req the JSON array of actions to be performed
 * \param[in] json_res the response body, as JSON
 * \param[in] hdl the action handler; callable of signature DependsOutcome(const rapidjson::Value&)
 **/
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