//
// Created by hugo on 30.01.19.
//
#pragma once

#include <algorithm>
#include <type_traits>

template<typename E>
constexpr inline decltype(auto) to_integral(E e) { return static_cast<typename std::underlying_type<E>::type>(e); }

constexpr auto pow10(std::size_t n) {
    auto ret = 1u;
    while (n--)
        ret *= 10;
    return ret;
}

constexpr unsigned strntou(const char *str, std::size_t len) {
    auto ret = 0u;
    for (auto i = 0u; i < len; ++i)
        ret += (str[i] - '0') * pow10(len - i - 1);
    return ret;
}

template<typename Container, typename T>
unsigned reverse_search(const Container &c, const T &e) {
    const auto it = std::find(c.begin(), c.end(), e);
    return static_cast<unsigned>(std::distance(c.begin(), it));
}
