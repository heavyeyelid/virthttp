//
// Created by _as on 2019-01-31.
//

#pragma once

#include <optional>
#include <string>
#include <variant>
#include <gsl/gsl>
#include <libvirt/libvirt.h>

#include "fwd.hpp"

namespace virt {
using TypedParamValueType = std::variant<int, unsigned, long long, unsigned long long, double, bool,
                                         std::string>; // warning: 3x heavier
class TypedParameter : public std::pair<std::string, TypedParamValueType> {
    friend TypedParams;

    struct no_name_tag {};

  public:
    explicit TypedParameter(const virTypedParameter&);

    TypedParameter(const virTypedParameter&, no_name_tag);
};

class TypedParams {
    friend Connection;
    friend Domain;
    friend TypedParameter;

    struct Element;
    struct Iterator;

    class Element { // TODO implement std::variant's interface
        friend Iterator;

        virTypedParameterPtr underlying{};

        constexpr Element(virTypedParameterPtr underlying) noexcept : underlying(underlying) {}

      public:
    };

    virTypedParameterPtr underlying{};
    int size = 0;
    int capacity = 0;

  public:
    inline ~TypedParams() noexcept;

    void add(gsl::czstring<> name, int);

    void add(gsl::czstring<> name, unsigned);

    void add(gsl::czstring<> name, long long);

    void add(gsl::czstring<> name, unsigned long long);

    void add(gsl::czstring<> name, double);

    void add(gsl::czstring<> name, bool);

    void add(gsl::czstring<> name, gsl::czstring<>);

    void add(const TypedParameter&);

    template <typename T> T get(gsl::czstring<> name) const;

    template <typename T> T& get(gsl::czstring<> name);

    constexpr operator bool() const noexcept { return underlying != nullptr; }
};

class TypedParams::Iterator {
    virTypedParameterPtr it{};

    constexpr Iterator(virTypedParameterPtr it) noexcept : it(it) {}

  public:
    constexpr Iterator& operator++() noexcept { return ++it, *this; }
    constexpr Iterator operator++(int) noexcept { return {it++}; }
    constexpr Iterator& operator--() noexcept { return --it, *this; }
    constexpr Iterator operator--(int) noexcept { return {it++}; }
    constexpr Iterator& operator+=(int v) noexcept { return it += v, *this; }
    constexpr Iterator& operator-=(int v) noexcept { return it -= v, *this; }
    constexpr Iterator operator+(int v) const noexcept { return {it + v}; }
    constexpr Iterator operator-(int v) const noexcept { return {it - v}; }
    constexpr TypedParams::Element operator*() const noexcept { return TypedParams::Element{it}; }

    constexpr bool operator==(const Iterator& oth) const noexcept { return it == oth.it; }
    constexpr bool operator!=(const Iterator& oth) const noexcept { return it != oth.it; }
};

}