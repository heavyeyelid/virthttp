#include <algorithm>
#include "wrapper/depends.hpp"

bool check_depends(const rapidjson::Value& json, std::vector<DependsOutcome>& outcomes, JsonRes& json_res) noexcept {
    auto error = [&](auto... args) { return json_res.error(args...); };
    const auto it = json.FindMember("depends");
    if (it == json.MemberEnd())
        return true;

    const auto& json_deps = it->value;
    const auto curr_idx = outcomes.size();
    auto success_pred = [&](const auto& dep) {
        if (!dep.IsInt() || dep.GetInt() >= curr_idx || outcomes[dep.GetInt()] != DependsOutcome::SUCCESS)
            return outcomes.push_back(DependsOutcome::SKIPPED), false;
        return true;
    };
    if (json_deps.IsArray()) {
        const auto deps_arr = json_deps.GetArray();
        return std::all_of(deps_arr.begin(), deps_arr.end(), success_pred);
    }
    if (json_deps.IsInt()) {
        return success_pred(json_deps);
    }
    return error(0), outcomes.push_back(DependsOutcome::FAILURE), false;
}