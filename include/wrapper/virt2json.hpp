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

inline auto to_json(const char* cstr) -> boost::json::value { return {cstr}; }

inline auto to_json(UniqueZstring zstr) -> boost::json::value { return to_json(static_cast<const char*>(zstr)); }

using VirtWrappedDomainTime = decltype(*std::declval<virt::Domain>().getTime());

inline auto to_json(VirtWrappedDomainTime time) -> boost::json::value {
    return {{"seconds", static_cast<int64_t>(time.seconds)}, {"nanosec", static_cast<unsigned>(time.nanosec)}};
}

template <class T> auto get_to_json(std::optional<T> opt) -> boost::json::value { return to_json(std::move(*opt)); }
