#pragma once
#include <optional>
#include <string_view>
#include "../../detect.hpp"
#include "urlparser.hpp"

#define SUBQ_LIFT(mem_fn) [&](auto&&... args) noexcept(noexcept(mem_fn(std::forward<decltype(args)>(args)...))) \
                            -> decltype(mem_fn(std::forward<decltype(args)>(args)...)) \
                            { return mem_fn(std::forward<decltype(args)>(args)...); }
/**
 * \internal
 * \brief Extracts a flag from the HTTP target
 *
 * Extracts by tag a flag from a TargetParser.
 * The flag values must be comma-separated and only the first appearance of `tag` within the target is considered.
 * If the flag is not composable (i.e. `F{} | F{}` is ill-formed), only the first value is considered.
 *
 * @tparam F the type of flag; value `Empty` SFINAE'ed out
 * @param[in] target the target to read the query from
 * @param[in] tag the key of the query where the flag values are
 * @return std::optional<F> the flag, or std::nullopt if an error occurred when reading the flag values.
 *      Note that if `F` is not composable, only the first flag value will be read
 **/
template <class F, class = std::enable_if_t<!std::is_same_v<F, Empty>>>
constexpr auto target_get_composable_flag(const TargetParser& target, std::string_view tag) noexcept -> std::optional<F> {
    auto flags = F{};
    if (auto csv = target[tag]; !csv.empty()) {
        for (CSVIterator state_it{csv}; state_it != state_it.end(); ++state_it) {
            const auto v = F::from_string(*state_it);
            if (!v)
                return std::nullopt;
            if constexpr (test_sfinae([](auto f, auto g) { return f | g; }, F{}, F{}))
                flags |= *v;
            else
                return v;
        }
    }
    return {flags};
}

/**
 * \internal
 * Creates a closure which invokes callables in order so long they return `DependsOutcome::SKIPPED`
 *
 * \tparam Fcns (deduced from `depends`)
 * \param depends a parameter pack of callables which match `DependsOutcome(ClosureArgs...)`
 *          where `ClosureArgs` is the parameter list of the closure returned by this function
 * \return a closure of signature void(auto&&...)
 **/
template <class... Fcns> constexpr auto parameterized_depends_scope(Fcns&&... depends) noexcept {
    using Arr = std::tuple<Fcns...>; // pray for SFO ; wait for expansion statements
    return [&](auto&&... args) {
        DependsOutcome ret = DependsOutcome::SKIPPED;
        visit(Arr{std::forward<Fcns>(depends)...}, [&](auto&& f) {
            if (ret != DependsOutcome::SKIPPED)
                return;
            if (const auto ao = std::forward<decltype(f)>(f)(args...); ao != DependsOutcome::SKIPPED) {
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