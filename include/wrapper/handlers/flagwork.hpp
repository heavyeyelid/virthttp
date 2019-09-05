#pragma once
#include <optional>
#include <string_view>
#include "urlparser.hpp"

#define SUBQ_LIFT(mem_fn) [&](auto... args) { return mem_fn(args...); }

template <class F, class = std::enable_if_t<!std::is_same_v<F, Empty>>>
constexpr auto target_get_composable_flag(const TargetParser& target, std::string_view tag) noexcept {
    auto flags = F{};
    if (auto csv = target[tag]; !csv.empty()) {
        for (CSVIterator state_it{csv}; state_it != state_it.end(); ++state_it) {
            const auto v = F::from_string(*state_it);
            if (!v)
                return std::optional<F>{std::nullopt};
            if constexpr (test_sfinae([](auto f, auto g) { return f | g; }, F{}, F{}))
                flags |= *v;
            else
                flags = *v;
        }
    }
    return std::optional<F>{flags};
}

template <class... Fcns> constexpr auto parameterized_depends_scope(Fcns&&... depends) noexcept {
    using Arr = std::tuple<Fcns...>; // pray for SFO ; wait for expansion statements
    return [&](auto&&... args) {
        DependsOutcome ret = DependsOutcome::SKIPPED;
        visit(Arr{depends...}, [&](auto f) {
            if (ret != DependsOutcome::SKIPPED)
                return;
            if (const auto ao = f(args...); ao != DependsOutcome::SKIPPED) {
                ret = ao;
            }
        });
        return ret;
    };
};

namespace subq_impl {}

template <class F, class TJ, class TI> auto subquery(std::string_view name, std::string_view opt_tag, F&& lifted, TJ&& to_json, TI) noexcept {
    return [&, name, opt_tag](int sq_lev, const TargetParser& target, auto& res_val, auto&& error) -> DependsOutcome {
        if (target.getPathParts()[sq_lev] == name) {
            using Flag = typename TI::type;
            Flag flag{};
            const auto opt_flags = target_get_composable_flag<Flag>(target, opt_tag);
            if (!opt_flags)
                return error(301), DependsOutcome::FAILURE;
            flag = *opt_flags;
            auto&& [res, success] = to_json(lifted(flag));
            if (success)
                res_val = res;
        }
        return DependsOutcome::SKIPPED;
    };
}
template <class F, class TJ> auto subquery(std::string_view name, F&& lifted, TJ&& to_json) noexcept {
    return [&, name](int sq_lev, const TargetParser& target, auto& res_val, auto&& error) -> DependsOutcome {
        if (target.getPathParts()[sq_lev] == name) {
            auto&& [res, success] = to_json(lifted());
            return success ? (res_val = res, DependsOutcome::SUCCESS) : DependsOutcome::FAILURE;
        }
        return DependsOutcome::SKIPPED;
    };
}