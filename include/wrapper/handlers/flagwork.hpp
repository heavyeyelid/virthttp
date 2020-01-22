#pragma once
#include <optional>
#include <string_view>
#include "../../detect.hpp"
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

namespace subq_impl {

template <class T, class A> using ToJson = decltype(to_json(std::declval<T&&>(), std::declval<A&>()));
template <class T, class A> using GetToJson = decltype(get_to_json(std::declval<T&&>(), std::declval<A&>()));

template <class T, class A> constexpr decltype(auto) auto_serialize(T&& v, A& al) {
    using Value = std::remove_reference_t<T>;
    using Alloc = std::remove_reference_t<A>;
    if constexpr (nstd::is_detected_v<ToJson, Value, Alloc>)
        return to_json(std::move(v), al);
    if constexpr (nstd::is_detected_v<GetToJson, Value, Alloc>)
        return get_to_json(std::move(v), al);
    else
        static_assert(std::is_void_v<Value>, "T is not auto-serializable");
    UNREACHABLE;
}

} // namespace subq_impl

template <class F, class VC, class TJ, class TI>
auto subquery(std::string_view name, std::string_view opt_tag, TI, F&& lifted, VC&& valid_check, TJ&& to_json) noexcept {
    return [&, name, opt_tag](int sq_lev, const TargetParser& target, auto& res_val, auto& allocator, auto&& error) -> DependsOutcome {
        if (target.getPathParts()[sq_lev] == name) {
            using Flag = typename TI::type;
            Flag flag{};
            const auto opt_flags = target_get_composable_flag<Flag>(target, opt_tag);
            if (!opt_flags)
                return error(301), DependsOutcome::FAILURE;
            flag = *opt_flags;
            auto&& res = lifted(flag);
            return valid_check(res) ? (res_val = std::move(to_json(std::move(res), allocator)), DependsOutcome::SUCCESS) : DependsOutcome::FAILURE;
        }
        return DependsOutcome::SKIPPED;
    };
}
template <class F, class VC, class TJ> auto subquery(std::string_view name, F&& lifted, VC&& valid_check, TJ&& to_json) noexcept {
    return [&, name](int sq_lev, const TargetParser& target, auto& res_val, auto& allocator, auto&& error) -> DependsOutcome {
        if (target.getPathParts()[sq_lev] == name) {
            auto&& res = lifted();
            return valid_check(res) ? (res_val = std::move(to_json(std::move(res), allocator)), DependsOutcome::SUCCESS) : DependsOutcome::FAILURE;
        }
        return DependsOutcome::SKIPPED;
    };
}

template <class F, class VC> auto subquery(std::string_view name, F&& lifted, VC&& valid_check) noexcept -> decltype(auto) {
    return subquery(name, std::forward<F>(lifted), std::forward<VC>(valid_check),
                    [](auto&&... args) -> decltype(subq_impl::auto_serialize(args...)) { return subq_impl::auto_serialize(args...); });
}