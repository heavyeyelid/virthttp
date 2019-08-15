#pragma once
#include <string_view>
#include <boost/utility/string_view.hpp> // C++2a this include goes away long before using modules
#include <gsl/gsl>
#include "handlers/domain.hpp"
#include "wrapper/handlers/base.hpp"
#include "urlparser.hpp"

using namespace std::literals;

constexpr std::string_view bsv2stdsv(boost::string_view bsv) noexcept { return {bsv.data(), bsv.length()}; }
constexpr boost::string_view stdsv2bsv(std::string_view sv) noexcept { return {sv.data(), sv.length()}; }

template <class TPOUH, class KeysT, class FcnsT, class ListFcn> class Resolver {
    std::string_view type;
    KeysT skeys;
    FcnsT sfcns;
    ListFcn list_fcn;

  public:
    using O = typename TPOUH::First;
    using UH = typename TPOUH::Second;

  private:
    [[nodiscard]] constexpr std::pair<int, std::string_view> getSearchKey(const TargetParser& target) const noexcept {
        const auto key_start = "/libvirt/"sv.length() + type.size();
        const auto path = stdsv2bsv(target.getPath().substr(key_start)); // C++2a: use std::string_view all the way
        if (path.empty())
            return {-1, ""};
        const auto it = cexpr::find_if(skeys.begin(), skeys.end(), [=](std::string_view sv) {
            auto l_path = path;
            if (!l_path.starts_with('/'))
                return false;
            l_path.remove_prefix(1);
            if (!l_path.starts_with(stdsv2bsv(sv)))
                return false;
            l_path.remove_prefix(sv.size());
            return l_path.starts_with('/');
        });
        if (it == skeys.end())
            return {std::numeric_limits<int>::min(), nullptr};

        const auto idx = std::distance(skeys.begin(), it);
        const auto sk = skeys[idx];
        const auto s_val = bsv2stdsv(path.substr(0, sk.length() + 2)); // 2 because two forward-slashes
        if (s_val.empty())
            return {std::numeric_limits<int>::min(), nullptr};
        return {idx, s_val};
    }

  public:
    constexpr Resolver(TPOUH, std::string_view type, KeysT skeys, FcnsT sfcns, ListFcn list_fcn) noexcept
        : type(type), skeys(skeys), sfcns(sfcns), list_fcn(list_fcn) {}

    auto operator()(const TargetParser& target, HandlerContext& hc) const -> std::vector<O> {
        using Ret = std::vector<O>;
        auto error = [&](auto... args) { return hc.json_res.error(args...); };

        const auto [idx, search_value] = getSearchKey(target);
        if (search_value == nullptr)
            return error(-999), Ret{}; // Basically happens when: 1. there's an unknown key 2. there's a known key but no value

        Ret ret;
        if (idx >= 0) {
            if (auto dom_res = sfcns[idx](hc, search_value); dom_res)
                ret.emplace_back(std::move(dom_res));
            else
                error(100);
        } else {
            const auto flags_opt = UH{hc}.search_all_flags(target);
            if (!flags_opt)
                return error(-999), Ret{}; // Happens when flags cause failure
            const auto flags = *flags_opt;
            auto list_res = list_fcn(hc, flags);
            std::move(list_res.begin(), list_res.end(), std::back_inserter(ret));
        }
        return ret;
    }
};