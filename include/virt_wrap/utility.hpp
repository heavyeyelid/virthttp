//
// Created by _as on 2019-01-31.
//

#pragma once

#include <algorithm>

// From C++ Weekly - Ep 134 (Jason Turner) :
// https://www.youtube.com/watch?v=EsUmnLgz8QY

template<typename... Base>
struct Visitor : Base ... {
    using Base::operator()...;
};

template<typename... T> Visitor(T...) -> Visitor<T...>;