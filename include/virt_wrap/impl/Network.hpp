#pragma once

#include <cstdlib>
#include <memory>
#include <vector>
#include <libvirt/libvirt.h>
#include "../Connection.hpp"
#include "../Network.hpp"
#include "../utility.hpp"

namespace virt {

constexpr inline Network::Network(Network&& net) noexcept : underlying(net.underlying) { net.underlying = nullptr; }

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

[[nodiscard]] inline UniqueZstring Network::getBridgeName() const noexcept { return UniqueZstring(virNetworkGetBridgeName(underlying)); }

[[nodiscard]] inline Connection Network::getConnect() const noexcept {
    const auto res = virNetworkGetConnect(underlying);
    if (res)
        virConnectRef(res);
    return Connection{res};
}

[[nodiscard]] inline passive<const char*> Network::getName() const noexcept { return virNetworkGetName(underlying); }

[[nodiscard]] inline std::string Network::extractName() const noexcept { return {getName()}; }

[[nodiscard]] inline auto Network::getUUID() const noexcept {
    std::array<unsigned char, VIR_UUID_BUFLEN> ret{};
    return virNetworkGetUUID(underlying, ret.data()) == 0 ? std::optional(ret) : std::nullopt;
}

[[nodiscard]] inline auto Network::getUUIDString() const noexcept -> std::optional<std::array<char, VIR_UUID_STRING_BUFLEN>> {
    std::array<char, VIR_UUID_STRING_BUFLEN> ret{};
    return virNetworkGetUUIDString(underlying, ret.data()) == 0 ? std::optional(ret) : std::nullopt;
}

[[nodiscard]] inline auto Network::extractUUIDString() const -> std::string {
    std::string ret{};
    ret.resize(VIR_UUID_STRING_BUFLEN);
    if (virNetworkGetUUIDString(underlying, ret.data()) != 0)
        return {};
    ret.resize(std::strlen(ret.data()));
    ret.shrink_to_fit();
    return ret;
}

[[nodiscard]] inline UniqueZstring Network::getXMLDesc(enums::network::XMLFlags flags) const noexcept {
    return UniqueZstring{virNetworkGetXMLDesc(underlying, to_integral(flags))};
}

[[nodiscard]] inline std::string Network::extractXMLDesc(enums::network::XMLFlags flags) const noexcept {
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
    std::for_each(lease_arr, lease_arr + res, [&](virNetworkDHCPLeasePtr v) {
        ret.push_back(*v);
        virNetworkDHCPLeaseFree(v);
    });
    freeany(lease_arr);
    return ret;
}
[[nodiscard]] inline auto Network::extractDHCPLeases(std::string mac) const -> std::optional<std::vector<virNetworkDHCPLease>> {
    return extractDHCPLeases(mac.empty() ? nullptr : mac.c_str());
}

inline bool Network::setAutostart(bool autostart) noexcept { return virNetworkSetAutostart(underlying, autostart) == 0; }

[[nodiscard]] inline TFE Network::getAutostart() const noexcept {
    int v;
    const auto res = virNetworkGetAutostart(underlying, &v);
    return TFE{res == 0 ? v : -1};
}

inline bool Network::create() noexcept { return virNetworkCreate(underlying) == 0; }

inline bool Network::destroy() noexcept { return virNetworkDestroy(underlying) == 0; }

inline bool Network::undefine() noexcept { return virNetworkUndefine(underlying) == 0; }

inline Network Network::createXML(Connection& conn, gsl::czstring<> xml) { return Network{virNetworkCreateXML(conn.underlying, xml)}; }
inline Network Network::defineXML(Connection& conn, gsl::czstring<> xml) { return Network{virNetworkDefineXML(conn.underlying, xml)}; }

} // namespace virt