//
// Created by _as on 2019-02-01.
//

#pragma once

#include <libvirt/libvirt.h>
#include <lift.hpp>
#include "../TypesParam.hpp"
#include "../utility.hpp"

namespace virt {
TypedParameter::TypedParameter(const virTypedParameter& from, virt::TypedParameter::no_name_tag) {
    switch (from.type) {
    case VIR_TYPED_PARAM_INT:
        second.emplace<int>(from.value.i);
        break;
    case VIR_TYPED_PARAM_UINT:
        second.emplace<unsigned>(from.value.ui);
        break;
    case VIR_TYPED_PARAM_LLONG:
        second.emplace<long long>(from.value.l);
        break;
    case VIR_TYPED_PARAM_ULLONG:
        second.emplace<unsigned long long>(from.value.ul);
        break;
    case VIR_TYPED_PARAM_DOUBLE:
        second.emplace<double>(from.value.d);
        break;
    case VIR_TYPED_PARAM_BOOLEAN:
        second.emplace<bool>(from.value.b);
        break;
    case VIR_TYPED_PARAM_STRING:
        second.emplace<std::string>(from.value.s);
        break;
    default:
        throw std::runtime_error{"Non-standard type-parameter"};
    }
}

TypedParameter::TypedParameter(const virTypedParameter& from) : TypedParameter(from, no_name_tag{}) { first = +from.field; }

template <class Container> TypedParams::TypedParams(Container cont) : needs_free(true) {
    for (const TypedParameter& tp : cont)
        add(tp);
}

TypedParams::~TypedParams() noexcept {
    if (underlying && needs_free)
        virTypedParamsFree(underlying, size);
    if (underlying && needs_dealloc)
        delete[] underlying;
}

void TypedParams::add(gsl::czstring<> name, int i) { virTypedParamsAddInt(&underlying, &size, &capacity, name, i); }

void TypedParams::add(gsl::czstring<> name, unsigned u) { virTypedParamsAddUInt(&underlying, &size, &capacity, name, u); }

void TypedParams::add(gsl::czstring<> name, long long ll) { virTypedParamsAddLLong(&underlying, &size, &capacity, name, ll); }

void TypedParams::add(gsl::czstring<> name, unsigned long long ull) { virTypedParamsAddULLong(&underlying, &size, &capacity, name, ull); }

void TypedParams::add(gsl::czstring<> name, double d) { virTypedParamsAddDouble(&underlying, &size, &capacity, name, d); }

void TypedParams::add(gsl::czstring<> name, bool b) { virTypedParamsAddBoolean(&underlying, &size, &capacity, name, b); }

void TypedParams::add(gsl::czstring<> name, gsl::czstring<> czs) { virTypedParamsAddString(&underlying, &size, &capacity, name, czs); }

void TypedParams::add(gsl::czstring<> name, const std::string& s) { virTypedParamsAddString(&underlying, &size, &capacity, name, s.c_str()); }

void TypedParams::add(const TypedParameter& tp) {
    const auto name = tp.first.data();
    std::visit([&, name](auto&& p) { this->add(name, p); }, tp.second);
}

template <typename T> T TypedParams::get(gsl::czstring<> name) const {
    const auto it = std::find_if(underlying, underlying + size, [&](const virTypedParameter& tp) { return std::strcmp(name, tp.field) == 0; });
    return std::get<T>(TypedParameter{it, TypedParameter::no_name_tag{}}.second);
}

template <typename T> T& TypedParams::get(gsl::czstring<> name) {
    auto it = std::find_if(underlying, underlying + size, [&](const virTypedParameter& tp) { return std::strcmp(name, tp.field) == 0; });
    return std::get<T>(TypedParameter{it, TypedParameter::no_name_tag{}}.second);
}
}