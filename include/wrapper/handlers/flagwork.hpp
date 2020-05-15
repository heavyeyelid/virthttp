#pragma once
#include <optional>
#include <string_view>
#include "../../detect.hpp"
#include "urlparser.hpp"

/**
 * @file
 * Utilities for making subqueries work and have as many parameters as possible be deduced
 */

/**
 * @param mem_fn a reference to the function to be lifted
 **/
#define SUBQ_LIFT(mem_fn)                                                                                                                            \
    [&](auto&&... args) noexcept(noexcept(mem_fn(std::forward<decltype(args)>(args)...))) -> decltype(                                               \
                                                                                              mem_fn(std::forward<decltype(args)>(args)...)) {       \
        return mem_fn(std::forward<decltype(args)>(args)...);                                                                                        \
    }

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
 * @return (`std::optional<F>`) the flag, or `std::nullopt` if an error occurred when reading the flag values.
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
 * \param[in] depends a parameter pack of callables which all match the same `DependsOutcome(auto&&...)`
 * \return (`auto`) a closure of signature void(auto&&...)
 **/
template <class... Fcns> constexpr auto parameterized_depends_scope(Fcns&&... depends) noexcept((std::is_nothrow_move_constructible_v<Fcns> && ...)) {
    using Arr = std::tuple<Fcns...>; // pray for vectorisation; wait for expansion statements
    return [&](auto&&... args) -> DependsOutcome {
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

/**
 * Subquerying implementation details
 **/
namespace subq_impl {

/**
 * \internal
 * SFINAE detection helper for the `to_json` function overload set
 *
 * \tparam T the type of the object to be serialized
 * \tparam A the allocator type
 **/
template <class T, class A> using ToJson = decltype(to_json(std::declval<T&&>(), std::declval<A&>()));
/**
 * \internal
 * SFINAE detection helper for the `get_to_json` function overload set
 *
 * \tparam T the type of the object to be serialized
 * \tparam A the allocator type
 **/
template <class T, class A> using GetToJson = decltype(get_to_json(std::declval<T&&>(), std::declval<A&>()));

/**
 * \internal
 * SFINAE detection wrapper to select the correct serialization function
 * The function template instanciation will fail to compile by `static_assert` if the object has no way of being automatically serialized
 *
 * \tparam T, A (deduced)
 * \param[in] v the object to be JSON-serialized; perfect-forwarded
 * \param[in] al a reference to the allocator to be used during serialization
 * \return (`decltype(auto)`) the perfectly forwarded result of the serialization function call
 **/
template <class T, class A> constexpr decltype(auto) auto_serialize(T&& v, A& al) {
    using Value = std::remove_reference_t<T>;
    using Alloc = std::remove_reference_t<A>;
    if constexpr (nstd::is_detected_v<ToJson, Value, Alloc>)
        return to_json(std::move(v), al);
    else if constexpr (nstd::is_detected_v<GetToJson, Value, Alloc>)
        return get_to_json(std::move(v), al);
    else
        static_assert(std::is_void_v<Value>, "T is not auto-serializable");
    UNREACHABLE;
}

/**
 * \internal
 * \brief subquery overload for functions taking a single flag and without automatically deduced JSON serialization
 *
 * Note: all three callable parameters are perfectly forwarded to their point of call
 *
 * \tparam TI, F, VC, TJ (deduced)
 * \param[in] name the name of the subquery
 * \param[in] opt_tag the key of the flag value(s) in the query string
 * \param[in] ti a `TypeIdentity<Flag>` object where `Flag` is the type of flag that `lifted` takes as parameter
 * \param[in] lifted the lifted overload set or function
 * \param[in] valid_check a callable used to check the result of the call to `lifted`; must have a return type contextually convertible to `bool`
 * \param[in] to_json a callable used to serialize the result of the call to `lifted` when `valid_check` has returned an contextual equivalent of
 *              `true`
 * \return a callable able to process the specified subquery
 **/
template <class TI, class F, class VC, class TJ>
auto subquery(std::string_view name, std::string_view opt_tag, [[maybe_unused]] TI ti, F&& lifted, VC&& valid_check, TJ&& to_json) noexcept(
    std::is_nothrow_move_constructible_v<F>&& std::is_nothrow_move_constructible_v<VC>&& std::is_nothrow_move_constructible_v<TJ>) {
    return [name, opt_tag, lifted = std::forward<F>(lifted), valid_check = std::forward<VC>(valid_check), to_json = std::forward<TJ>(to_json)](
               int sq_lev, const TargetParser& target, auto& res_val, auto& allocator, auto&& error) -> DependsOutcome {
        if (target.getPathParts()[sq_lev] == name) {
            using Flag = typename TI::type;
            Flag flag{};
            if constexpr (std::is_same_v<Flag, std::string_view> || std::is_same_v<Flag, std::string>)
                flag = target[opt_tag];
            else {
                const auto opt_flags = target_get_composable_flag<Flag>(target, opt_tag);
                if (!opt_flags)
                    return error(301), DependsOutcome::FAILURE;
                flag = *opt_flags;
            }
            auto&& res = lifted(std::move(flag));
            return valid_check(res) ? (res_val = std::move(to_json(std::move(res), allocator)), DependsOutcome::SUCCESS) : DependsOutcome::FAILURE;
        }
        return DependsOutcome::SKIPPED;
    };
}

/**
 * \internal
 * \brief subquery overload for functions taking no flag and with automatically deduced JSON serialization
 *
 * Note: both callable parameters are perfectly forwarded to their point of call
 *
 * \tparam F, VC, TI (deduced)
 * \param[in] name the name of the subquery
 * \param[in] opt_tag the key of the flag value(s) in the query string
 * \param[in] ti a `TypeIdentity<Flag>` object where `Flag` is the type of flag that `lifted` takes as parameter
 * \param[in] lifted the lifted overload set or function
 * \param[in] valid_check a callable used to check the result of the call to `lifted`; must have a return type contextually convertible to `bool`
 * \return a callable able to process the specified subquery
 **/
template <class F, class VC, class TI>
auto subquery(std::string_view name, std::string_view opt_tag, TI ti, F&& lifted,
              VC&& valid_check) noexcept(std::is_nothrow_move_constructible_v<F>&& std::is_nothrow_move_constructible_v<VC>) -> decltype(auto) {
    return subquery(name, opt_tag, ti, std::forward<F>(lifted), std::forward<VC>(valid_check),
                    [](auto&&... args) -> decltype(subq_impl::auto_serialize(args...)) { return subq_impl::auto_serialize(args...); });
}

/**
 * \internal
 * \brief subquery overload for functions taking no flag and without automatically deduced JSON serialization
 *
 * Note: all three callable parameters are perfectly forwarded to their point of call
 *
 * \tparam F, VC, TJ (deduced)
 * \param[in] name the name of the subquery
 * \param[in] lifted the lifted overload set or function
 * \param[in] valid_check a callable used to check the result of the call to `lifted`; must have a return type contextually convertible to `bool`
 * \param[in] to_json a callable used to serialize the result of the call to `lifted` when `valid_check` has returned an contextual equivalent of
 *              `true`
 * \return a callable able to process the specified subquery
 **/
template <class F, class VC, class TJ, class = std::enable_if_t<!std::is_same_v<F, std::string_view>>>
auto subquery(std::string_view name, F&& lifted, VC&& valid_check, TJ&& to_json) noexcept(
    std::is_nothrow_move_constructible_v<F>&& std::is_nothrow_move_constructible_v<VC>&& std::is_nothrow_move_constructible_v<TJ>) {
    return [&, name, lifted = std::forward<F>(lifted), valid_check = std::forward<VC>(valid_check), to_json = std::forward<TJ>(to_json)](
               int sq_lev, const TargetParser& target, auto& res_val, auto& allocator, auto&& error) -> DependsOutcome {
        if (target.getPathParts()[sq_lev] == name) {
            auto&& res = lifted();
            return valid_check(res) ? (res_val = std::move(to_json(std::move(res), allocator)), DependsOutcome::SUCCESS) : DependsOutcome::FAILURE;
        }
        return DependsOutcome::SKIPPED;
    };
}

/**
 * \internal
 * \brief subquery overload for functions taking no flag and with automatically deduced JSON serialization
 *
 * Note: both callable parameters are perfectly forwarded to their point of call
 *
 * \tparam F, VC (deduced)
 * \param[in] name the name of the subquery
 * \param[in] lifted the lifted overload set or function
 * \param[in] valid_check a callable used to check the result of the call to `lifted`; must have a return type contextually convertible to `bool`
 * \return a callable able to process the specified subquery
 **/
template <class F, class VC, class = std::enable_if_t<!std::is_same_v<F, std::string_view>>>
auto subquery(std::string_view name, F&& lifted,
              VC&& valid_check) noexcept(std::is_nothrow_move_constructible_v<F>&& std::is_nothrow_move_constructible_v<VC>) -> decltype(auto) {
    return subquery(name, std::forward<F>(lifted), std::forward<VC>(valid_check),
                    [](auto&&... args) -> decltype(subq_impl::auto_serialize(args...)) { return subq_impl::auto_serialize(args...); });
}

} // namespace subq_impl

/**
 * \internal
 * \brief Perfect-forwarding lifted subq_impl::subquery
 *
 * Lifts a set of closure factories, where said closures process the subquery described by `args`.
 * The resulting closure upon invocation of this lifting lambda must be of the signature `DependsOutcome(int sq_lev, const TargetParser& target, auto&
 *res_val, auto& allocator, auto&& error)`, where:
 *   - [in] `sq_lev` is the target path part index where the subquery to look for is located
 *   - [in] `target` is the target parser object containing the currently parsed URI target
 *   - [out] `res_val` is the place where to put the JSON result to be sent back to the client
 *   - [in] `allocator` is the JSON allocator to be used when serializing
 *   - [in] `error` is a callable used for error reporting
 *
 * \param args the subquery description; see the different versions of subq_impl::subquery to know the possible description formats
 **/
constexpr auto subquery = [](auto&&... args) noexcept(noexcept(subq_impl::subquery(std::forward<decltype(args)>(args)...)))
    -> decltype(subq_impl::subquery(std::forward<decltype(args)>(args)...)) { return subq_impl::subquery(std::forward<decltype(args)>(args)...); };