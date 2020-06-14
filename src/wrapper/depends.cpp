#include <algorithm>
#include "wrapper/depends.hpp"

bool check_depends(const boost::json::value& json, std::vector<DependsOutcome>& outcomes, JsonRes& json_res) noexcept {
    auto error = [&](auto... args) { return json_res.error(args...); };
    if (!json.is_object())
        return true;
    const auto& jobj = json.get_object();
    const auto it = jobj.find("depends");
    if (it == jobj.end())
        return true;

    const auto& json_deps = it->value();
    const auto curr_idx = outcomes.size();
    auto success_pred = [&](const boost::json::value& dep) {
        boost::json::error_code ec;
        if (const auto idx = boost::json::number_cast<std::uint64_t>(dep, ec); ec || idx >= curr_idx || outcomes[idx] != DependsOutcome::SUCCESS)
            return outcomes.push_back(DependsOutcome::SKIPPED), false;
        return true;
    };
    if (json_deps.is_array()) {
        const auto& deps_arr = json_deps.get_array();
        return std::all_of(deps_arr.begin(), deps_arr.end(), success_pred);
    }
    if (json_deps.is_uint64()) {
        return success_pred(json_deps);
    }
    return error(0), outcomes.push_back(DependsOutcome::FAILURE), false;
}