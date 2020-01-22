#pragma once
#include <libvirt/libvirt.h>
#include <rapidjson/pointer.h>
#include "virt_wrap/impl/TypedParams.hpp"

template <class JAllocator> auto to_json(const virt::TypedParams& tp, JAllocator&& jalloc) noexcept -> rapidjson::Value {
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

template <class JAllocator> auto to_json(const char* cstr, JAllocator&& jalloc) -> rapidjson::Value { return {cstr, jalloc}; }

template <class JAllocator> auto to_json(UniqueZstring zstr, JAllocator&& jalloc) -> rapidjson::Value {
    return to_json(static_cast<const char*>(zstr), std::forward<JAllocator>(jalloc));
}

using VirtWrappedDomainTime = decltype(*std::declval<virt::Domain>().getTime());

template <class JAllocator> auto to_json(VirtWrappedDomainTime time, JAllocator&& jalloc) -> rapidjson::Value {
    rapidjson::Value ret{};
    ret.SetObject();
    ret.AddMember("seconds", static_cast<int64_t>(time.seconds), jalloc);
    ret.AddMember("nanosec", static_cast<unsigned>(time.nanosec), jalloc);
    return ret;
}

template <class T, class JAllocator> auto get_to_json(std::optional<T> opt, JAllocator&& jalloc) -> rapidjson::Value {
    return to_json(std::move(*opt), std::forward<JAllocator>(jalloc));
}
