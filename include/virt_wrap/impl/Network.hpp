#pragma once

#include <cstdlib>
#include <memory>
#include <vector>
#include <libvirt/libvirt.h>
#include "../Connection.hpp"
#include "../Network.hpp"
#include "../utility.hpp"

namespace virt {

inline Network& Network::operator=(Network&& net) noexcept {
    this->~Network();
    underlying = net.underlying;
    net.underlying = nullptr;
    return *this;
}

inline Network::~Network() noexcept {
    if (*this)
        virNetworkFree(underlying);
}

[[nodiscard]] inline auto Network::getBridgeName() const noexcept {
    return std::unique_ptr<char[], void (*)(char*)>(virNetworkGetBridgeName(underlying), freeany<char[]>);
}

[[nodiscard]] inline Connection Network::getConnect() const noexcept { return Connection{virNetworkGetConnect(underlying)}; }

[[nodiscard]] inline passive<const char*> Network::getName() const noexcept { return virNetworkGetName(underlying); }

[[nodiscard]] inline std::string Network::extractName() const noexcept { return {getName()}; }

[[nodiscard]] inline auto Network::getUUID() const noexcept {
    std::array<unsigned char, VIR_UUID_BUFLEN> ret{};
    return virNetworkGetUUID(underlying, ret.data()) == 0 ? std::optional(ret) : std::nullopt;
}

[[nodiscard]] inline auto Network::getUUIDString() const noexcept {
    std::array<char, VIR_UUID_STRING_BUFLEN> ret{};
    return virNetworkGetUUIDString(underlying, ret.data()) == 0 ? std::optional(ret) : std::nullopt;
}

[[nodiscard]] inline auto Network::getXMLDesc(Network::XMLFlags flags) const noexcept {
    return std::unique_ptr<char[], void (*)(char*)>(virNetworkGetXMLDesc(underlying, to_integral(flags)), freeany<char[]>);
}

[[nodiscard]] inline std::string Network::extractXMLDesc(Network::XMLFlags flags) const noexcept {
    const auto res = virNetworkGetXMLDesc(underlying, to_integral(flags));
    const auto ret = std::string{res};
    freeany(res);
    return ret;
}

[[nodiscard]] inline TFE Network::isActive() const noexcept { return TFE{virNetworkIsActive(underlying)}; }

[[nodiscard]] inline TFE Network::isPersistent() const noexcept { return TFE{virNetworkIsPersistent(underlying)}; }

[[nodiscard]] inline auto Network::getDHCPLeases(gsl::czstring<> mac) const noexcept {
    using RetType = std::optional<std::unique_ptr<virNetworkDHCPLeasePtr[], void (*)(virNetworkDHCPLeasePtr*)>>;
    virNetworkDHCPLeasePtr* lease_arr;
    auto res = virNetworkGetDHCPLeases(underlying, mac, &lease_arr, 0);
    if (res == -1)
        return RetType{std::nullopt};
    return std::optional{std::unique_ptr<virNetworkDHCPLeasePtr[], void (*)(virNetworkDHCPLeasePtr*)>(lease_arr, [](auto arr) {
        auto it = arr;
        while (it++)
            virNetworkDHCPLeaseFree(*it);
        freeany(arr);
    })};
}

[[nodiscard]] inline auto Network::extractDHCPLeases(gsl::czstring<> mac) const -> std::optional<std::vector<virNetworkDHCPLease>> {
    virNetworkDHCPLeasePtr* lease_arr;
    auto res = virNetworkGetDHCPLeases(underlying, mac, &lease_arr, 0);
    if (res == -1)
        return std::nullopt;
    std::vector<virNetworkDHCPLease> ret{};
    ret.reserve(res);
    std::for_each(*lease_arr, *lease_arr + res, [&](virNetworkDHCPLease v) {
        ret.push_back(v);
        virNetworkDHCPLeaseFree(&v);
    });
    freeany(lease_arr);
    return ret;
}

inline bool Network::setAutostart(bool autostart) noexcept { return virNetworkSetAutostart(underlying, autostart) == 0; }

[[nodiscard]] inline TFE Network::getAutostart() const noexcept {
    int v;
    const auto res = virNetworkGetAutostart(underlying, &v);
    return TFE{res == 0 ? v : -1};
}

inline bool Network::create() noexcept { return virNetworkCreate(underlying) == 0; }

bool Network::destroy() noexcept { return virNetworkDestroy(underlying) == 0; }

bool Network::undefine() noexcept { return virNetworkUndefine(underlying) == 0; }

inline Network Network::createXML(Connection& conn, gsl::czstring<> xml) { return Network{virNetworkCreateXML(conn.underlying, xml)}; }
inline Network Network::defineXML(Connection& conn, gsl::czstring<> xml) { return Network{virNetworkDefineXML(conn.underlying, xml)}; }

}