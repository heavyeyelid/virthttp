//
// Created by _as on 2019-01-31.
//

#pragma once

#include <type_traits>

template <typename E> constexpr inline auto to_integral(E e) -> std::underlying_type_t<E> { return static_cast<std::underlying_type_t<E>>(e); }