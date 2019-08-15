//
// Created by hugo on 30.01.19.
//
#pragma once

#include <algorithm>
#include <tuple>
#include <type_traits>
#include "virt_wrap/utility.hpp"

template <class T> struct TypeVal { using Type = T; };
template <class T> TypeVal<T> t_{};

template <class T, class U> struct TypePair {
    using First = T;
    using Second = U;
};

template <class T, class U> TypePair<T, U> tp{};

template <typename E> constexpr inline decltype(auto) to_integral(E e) { return static_cast<typename std::underlying_type<E>::type>(e); }

constexpr auto pow10(std::size_t n) {
    auto ret = 1u;
    while (n--)
        ret *= 10;
    return ret;
}

constexpr unsigned strntou(const char* str, std::size_t len) {
    auto ret = 0u;
    for (auto i = 0u; i < len; ++i)
        ret += (str[i] - '0') * pow10(len - i - 1);
    return ret;
}

template <typename Container, typename T> unsigned reverse_search(const Container& c, const T& e) {
    const auto it = std::find(c.begin(), c.end(), e);
    return static_cast<unsigned>(std::distance(c.begin(), it));
}

template <typename T, typename V, size_t... I> void visit_impl(T&& t, V&& v, std::index_sequence<I...>) { (..., v(std::get<I>(t))); }

template <typename T, typename V> void visit(T&& t, V&& v) {
    visit_impl(std::forward<T>(t), std::forward<V>(v), std::make_index_sequence<std::tuple_size<typename std::decay<T>::type>::value>());
}