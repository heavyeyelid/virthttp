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
    return std::move(ret);
}

template <class JAllocator> auto to_json(virNetworkDHCPLease lease, JAllocator&& jalloc) -> rapidjson::Value {
    rapidjson::Value json_lease;
    json_lease.AddMember("type", lease.type, jalloc);
    json_lease.AddMember("client-id", to_json(lease.clientid, jalloc), jalloc);
    json_lease.AddMember("expiry-time", static_cast<std::int64_t>(lease.expirytime), jalloc);
    json_lease.AddMember("hostname", to_json(lease.hostname, jalloc), jalloc);
    json_lease.AddMember("iaid", to_json(lease.iaid, jalloc), jalloc);
    json_lease.AddMember("interface", to_json(lease.iface, jalloc), jalloc);
    json_lease.AddMember("ip-address", to_json(lease.ipaddr, jalloc), jalloc);
    json_lease.AddMember("mac-address", to_json(lease.mac, jalloc), jalloc);
    json_lease.AddMember("prefix", lease.prefix, jalloc);
    return json_lease;
}

auto to_json(TFE tfe) -> rapidjson::Value {
    assert(!tfe.err());
    rapidjson::Value v;
    v.SetBool(static_cast<bool>(tfe));
    return v;
}

template <class T, class JAllocator> auto to_json(std::vector<T> vec, JAllocator&& jalloc) -> rapidjson::Value {
    rapidjson::Value arr;
    arr.SetArray();
    for (auto&& element : std::move(vec))
        arr.PushBack(to_json(std::move(element), jalloc), jalloc);
    return std::move(arr);
}

template <class T, class JAllocator> auto get_to_json(std::optional<T> opt, JAllocator&& jalloc) -> rapidjson::Value {
    return to_json(std::move(*opt), std::forward<JAllocator>(jalloc));
}
