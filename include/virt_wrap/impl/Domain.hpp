//
// Created by _as on 2019-02-01.
//

#pragma once

#include <algorithm>
#include <libvirt/libvirt.h>
#include "../Domain.hpp"
#include "../TypesParam.hpp"
#include "../type_ops.hpp"
#include "Connection.hpp"

namespace virt {
constexpr inline Domain::Domain(virDomainPtr ptr) noexcept : underlying(ptr) {}
inline Domain& Domain::operator=(Domain&& d) noexcept {
    this->~Domain();
    underlying = d.underlying;
    return *this;
}

inline Domain::~Domain() noexcept {
    if (*this)
        virDomainFree(underlying);
}

constexpr inline Domain::operator bool() const noexcept { return underlying != nullptr; }

[[nodiscard]] inline Domain Domain::createXML(Connection& c, gsl::czstring<> xml, CreateFlags flags) {
    return Domain{virDomainCreateXML(c.underlying, xml, to_integral(flags))};
}

bool inline Domain::create() noexcept {
    return virDomainCreate(underlying) == 0;
}

[[nodiscard]] inline Domain::Info Domain::getInfo() const noexcept {
    virDomainInfo info;
    virDomainGetInfo(underlying, &info);
    return info;
}

[[nodiscard]] inline gsl::czstring<> Domain::getName() const noexcept { return virDomainGetName(underlying); }

[[nodiscard]] inline unsigned Domain::getID() const noexcept { return virDomainGetID(underlying); }

[[nodiscard]] inline auto Domain::getUUID() const {
    using RetType = std::array<unsigned char, VIR_UUID_BUFLEN>;
    RetType ret{};
    const auto res = virDomainGetUUID(underlying, ret.data());
    if (res < 0)
        throw std::runtime_error{"virDomainGetUUID"};
    return ret;
}

[[nodiscard]] std::string Domain::getUUIDString() const noexcept {
    std::string ret{};
    ret.resize(VIR_UUID_STRING_BUFLEN);
    virDomainGetUUIDString(underlying, ret.data());
    ret.resize(std::strlen(ret.data()));
    ret.shrink_to_fit();
    return ret;
}

[[nodiscard]] inline  auto Domain::getOSType() const { return std::unique_ptr<char[], void (*)(char*)>{virDomainGetOSType(underlying), freeany<char[]>}; }

[[nodiscard]] inline unsigned long Domain::getMaxMemory() const noexcept { return virDomainGetMaxMemory(underlying); }

inline bool Domain::setMaxMemory(unsigned long mem) { return virDomainSetMaxMemory(underlying, mem) == 0; }

inline bool Domain::setMemory(unsigned long mem) { return virDomainSetMemory(underlying, mem) == 0; }

[[nodiscard]] inline bool Domain::isActive() const noexcept { return virDomainIsActive(underlying) != 0; }

inline void Domain::reboot(Domain::ShutdownFlags flags) { virDomainReboot(underlying, to_integral(flags)); }

inline void Domain::rename(gsl::czstring<> name) { virDomainRename(underlying, name, 0); }

inline void Domain::reset() { virDomainReset(underlying, 0); }

inline bool Domain::resume() noexcept { return virDomainResume(underlying) == 0; }

inline void Domain::setAutoStart(bool as) { virDomainSetAutostart(underlying, as ? 1 : 0); }

inline bool Domain::shutdown() noexcept { return virDomainShutdown(underlying) == 0; }

inline bool Domain::shutdown(Domain::ShutdownFlags flags) noexcept { return virDomainShutdownFlags(underlying, to_integral(flags)) == 0; }

inline void Domain::suspend() { virDomainSuspend(underlying); }

inline void Domain::destroy() { virDomainDestroy(underlying); }

inline void Domain::undefine(UndefineFlags flags) { virDomainUndefineFlags(underlying, to_integral(flags)); }

inline Domain::Stats::Record::Record(const virDomainStatsRecord& from) noexcept : dom(from.dom) {
    params.reserve(static_cast<std::size_t>(from.nparams));
    std::transform(from.params, from.params + from.nparams, std::back_inserter(params),
                   [](const virTypedParameter& tp) { return TypedParameter{tp}; });
}

[[nodiscard]] constexpr inline Domain::Stats::Types operator|(Domain::Stats::Types lhs, Domain::Stats::Types rhs) noexcept {
    return Domain::Stats::Types(to_integral(lhs) | to_integral(rhs));
}
}