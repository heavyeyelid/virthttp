#pragma once
#include <optional>
#include <string_view>
#include "urlparser.hpp"

template <class F, class FC> constexpr std::optional<F> target_get_composable_flag(const TargetParser& target, std::string_view tag) noexcept {
    auto flags = F{0};
    if (auto csv = target[tag]; !csv.empty()) {
        for (CSVIterator state_it{csv}; state_it != state_it.end(); ++state_it) {
            const auto v = FC{}[*state_it];
            if (!v)
                return std::nullopt;
            flags |= *v;
        }
    }
    return {flags};
}