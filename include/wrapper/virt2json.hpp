#pragma once
#include <boost/json.hpp>
#include <libvirt/libvirt.h>
#include "virt_wrap/impl/TypedParams.hpp"

inline auto to_json(const virt::TypedParams& tp) noexcept -> boost::json::value {
    boost::json::object ret;
    for (const virt::TypedParams::Element& v : tp) {
        switch (v.type()) {
        case VIR_TYPED_PARAM_INT:
            ret.emplace(v.name(), v.value().i);
            break;
        case VIR_TYPED_PARAM_UINT:
            ret.emplace(v.name(), v.value().ui);
            break;
        case VIR_TYPED_PARAM_LLONG:
            ret.emplace(v.name(), v.value().l);
            break;
        case VIR_TYPED_PARAM_ULLONG:
            ret.emplace(v.name(), v.value().ul);
            break;
        case VIR_TYPED_PARAM_DOUBLE:
            ret.emplace(v.name(), v.value().d);
            break;
        case VIR_TYPED_PARAM_BOOLEAN:
            ret.emplace(v.name(), v.value().b);
            break;
        case VIR_TYPED_PARAM_STRING:
            ret.emplace(v.name(), v.value().s);
            break;
        default:
            UNREACHABLE;
        }
    }
    return ret;
}

inline auto to_json(int i) -> boost::json::value { return {i}; }

inline auto to_json(const char* cstr) -> boost::json::value { return {cstr}; }

inline auto to_json(UniqueZstring zstr) -> boost::json::value { return to_json(static_cast<const char*>(zstr)); }

using VirtWrappedDomainTime = decltype(*std::declval<virt::Domain>().getTime());

auto to_json(VirtWrappedDomainTime time) -> boost::json::value {
    return boost::json::object{{"seconds", static_cast<int64_t>(time.seconds)}, {"nanosec", static_cast<unsigned>(time.nanosec)}
    };
}

auto to_json(virNetworkDHCPLease lease) -> boost::json::value {
    return boost::json::object{
        {"type", lease.type},
        {"client-id", to_json(lease.clientid)},
        {"expiry-time", static_cast<std::int64_t>(lease.expirytime)},
        {"hostname", to_json(lease.hostname)},
        {"iaid", to_json(lease.iaid)},
        {"interface", to_json(lease.iface)},
        {"ip-address", to_json(lease.ipaddr)},
        {"mac-address", to_json(lease.mac)},
        {"prefix", lease.prefix},
    };
}

auto to_json(TFE tfe) -> boost::json::value {
    assert(!tfe.err());
    return boost::json::object{static_cast<bool>(tfe)};
}

template <class T> auto to_json(std::vector<T> vec) -> boost::json::value {
    boost::json::array arr;
    for (auto&& element : std::move(vec))
        arr.push_back(to_json(std::move(element)));
    return std::move(arr);
}

template <class T> auto get_to_json(std::optional<T> opt) -> boost::json::value { return to_json(std::move(*opt)); }
