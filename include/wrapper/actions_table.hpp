#pragma once

#include <array>
#include <functional>
#include <string_view>
#include <gsl/gsl>
#include "cexpr_algs.hpp"
#include "depends.hpp"
#include "json2virt.hpp"
#include "json_utils.hpp"
#include "utils.hpp"

#define PM_LIFT(mem_fn) [&](auto... args) { return mem_fn(args...); }
#define PM_PREREQ(...) [&] { __VA_ARGS__ return DependsOutcome::SUCCESS; }

using namespace std::literals;

template <typename Flag>
constexpr const auto getFlag = [](const boost::json::string& json_flag, auto error) {
    if (auto v = Flag::from_string(json_flag); v)
        return std::optional{*v};
    return error(301), std::optional<Flag>{std::nullopt};
};
template <class F> auto getCombinedFlags(const boost::json::value& json_flag, JsonRes& json_res) noexcept -> std::optional<F> {
    auto error = [&](auto... args) { return json_res.error(args...), std::nullopt; };

    F flagset{};
    if (json_flag.is_array()) {
        const auto json_arr = json_flag.get_array();
        if constexpr (test_sfinae([](auto f) -> decltype(f | f) { UNREACHABLE; }, F{})) {
            for (const auto& json_el : json_arr) {
                if (!json_el.is_string())
                    return std::nullopt;
                const auto v = getFlag<F>(json_el, error);
                if (!v)
                    return std::nullopt;
                flagset |= *v;
            }
        } else {
            if (json_arr.size() > 1)
                return error(301);
            return {json_arr.empty() ? F{} : getFlag<F>(json_arr[0], error)};
        }
    } else if (json_flag.is_string()) {
        const auto v = getFlag<F>(json_flag, error);
        if (!v)
            return std::nullopt;
        flagset = *v;
    }
    return {flagset};
}

constexpr auto action_scope = [](auto&&... actions) {
    using Arr = std::array<std::function<DependsOutcome()>, sizeof...(actions)>; // pray for SFO ; wait for expansion statements
    for (auto&& action : Arr{actions...}) {
        if (const auto ao = action(); ao != DependsOutcome::SKIPPED)
            return ao;
    }
    return DependsOutcome::SKIPPED;
};

template <class CRTP, class Hdl> class NamedCallTable {
    [[nodiscard]] constexpr auto keys() const noexcept -> auto& { return static_cast<const CRTP&>(*this).keys; }
    [[nodiscard]] constexpr auto fcns() const noexcept -> auto& { return static_cast<const CRTP&>(*this).fcns; }

  public:
    constexpr Hdl operator[](std::string_view sv) const noexcept {
        const auto it = cexpr::find(keys().begin(), keys().end(), sv);
        if (it == keys().end())
            return nullptr;
        const auto idx = std::distance(keys().begin(), it);
        return fcns()[idx];
    }
};
