#pragma once
#include <string_view>
#include <boost/utility/string_view.hpp> // C++2a this include goes away long before using modules
#include <gsl/gsl>
#include "handlers/domain.hpp"
#include "wrapper/handlers/base.hpp"
#include "urlparser.hpp"

using namespace std::literals;

/**
 * \internal
 * Functor used to resolve the selected libvirt objects
 *
 * \tparam TPOUH TypePair<Object, UnawareHandlers>
 * \tparam KeysT constexpr-able ranged container of search keys
 * \tparam FcnsT constexpr-able ranged container of lifted search callables
 * \tparam ListFcn list-all callable type
 **/
template <class TPOUH, class KeysT, class FcnsT, class ListFcn> class Resolver {
    std::string_view type; ///< libvirt object class key in URIs
    KeysT skeys;           ///< Search keys
    FcnsT sfcns;           ///< Search callables
    ListFcn list_fcn;      ///< List-all callable

  public:
    using O = typename TPOUH::First;
    using UH = typename TPOUH::Second;

  private:
    [[nodiscard]] constexpr std::pair<int, std::string_view> getSearchKey(const TargetParser& target) const noexcept {
        const auto& path_parts = target.getPathParts();
        if (path_parts.size() < 3)
            return {-1, ""};
        if (path_parts.size() < 4)
            return {std::numeric_limits<int>::min(), std::string_view{}};

        const auto path_part = path_parts[2];
        const auto it = cexpr::find(skeys.begin(), skeys.end(), path_part);
        if (it == skeys.end())
            return {std::numeric_limits<int>::min(), std::string_view{}};

        const auto idx = std::distance(skeys.begin(), it);
        return {idx, path_parts[3]};
    }

  public:
    constexpr Resolver(TPOUH, std::string_view type, KeysT skeys, FcnsT sfcns, ListFcn list_fcn) noexcept
        : type(type), skeys(skeys), sfcns(sfcns), list_fcn(list_fcn) {}

    auto operator()(HandlerContext& hc) const -> std::vector<O> {
        const TargetParser& target = hc.target;
        using Ret = std::vector<O>;
        auto error = [&](auto... args) { return hc.json_res.error(args...); };

        const auto [idx, search_value] = getSearchKey(target);
        if (search_value.data() == nullptr)
            return error(101), Ret{}; // Basically happens when: 1. there's an unknown key 2. there's a known key but no value

        Ret ret;
        if (idx >= 0) {
            if (auto dom_res = sfcns[idx](hc, search_value); dom_res)
                ret.emplace_back(std::move(dom_res));
            else
                error(100);
        } else {
            const auto flags_opt = UH{hc}.search_all_flags(target);
            if (!flags_opt)
                return error(102), Ret{}; // Happens when flags cause failure
            const auto flags = *flags_opt;
            ret = list_fcn(hc, flags);
        }
        return ret;
    }
};