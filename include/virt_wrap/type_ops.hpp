//
// Created by _as on 2019-01-31.
//

#pragma once

#include <type_traits>

template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>> constexpr inline auto to_integral(E e) -> decltype(auto) {
    return static_cast<std::underlying_type_t<E>>(e);
}