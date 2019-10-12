#pragma once
#include <libvirt/libvirt.h>
#include <rapidjson/pointer.h>
#include "virt_wrap/impl/TypedParams.hpp"

template <class JAllocator> rapidjson::Value to_json(const virt::TypedParams& tp, JAllocator&& jalloc) noexcept {
    rapidjson::Value ret;
    for (const virt::TypedParams::Element& v : tp) {
        switch (v.type()) {
        case VIR_TYPED_PARAM_INT:
            ret.AddMember(rapidjson::Value(v.name(), jalloc), rapidjson::Value(v.value().i), jalloc);
            break;
        case VIR_TYPED_PARAM_UINT:
            ret.AddMember(rapidjson::Value(v.name(), jalloc), rapidjson::Value(v.value().ui), jalloc);
            break;
        case VIR_TYPED_PARAM_LLONG:
            ret.AddMember(rapidjson::Value(v.name(), jalloc), rapidjson::Value(int64_t{v.value().l}), jalloc);
            break;
        case VIR_TYPED_PARAM_ULLONG:
            ret.AddMember(rapidjson::Value(v.name(), jalloc), rapidjson::Value(uint64_t{v.value().ul}), jalloc);
            break;
        case VIR_TYPED_PARAM_DOUBLE:
            ret.AddMember(rapidjson::Value(v.name(), jalloc), rapidjson::Value(v.value().d), jalloc);
            break;
        case VIR_TYPED_PARAM_BOOLEAN:
            ret.AddMember(rapidjson::Value(v.name(), jalloc), rapidjson::Value(v.value().b), jalloc);
            break;
        case VIR_TYPED_PARAM_STRING:
            ret.AddMember(rapidjson::Value(v.name(), jalloc), rapidjson::Value(v.value().s, jalloc), jalloc);
            break;
        default:
            UNREACHABLE;
        }
    }
    return ret;
}