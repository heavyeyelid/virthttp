//
// Created by _as on 2019-01-31.
//

#pragma once

#include <type_traits>

template<typename E>
constexpr inline decltype(auto) to_integral(E e) { return static_cast<typename std::underlying_type<E>::type>(e); }