#pragma once
#include <libvirt/libvirt.h>
#include "../NodeDevice.hpp"

namespace virt {
constexpr inline NodeDevice::NodeDevice(virNodeDevicePtr ptr) noexcept : underlying(ptr) {}

constexpr inline NodeDevice::NodeDevice(NodeDevice&& nd) noexcept : underlying(nd.underlying) { nd.underlying = nullptr; }

inline NodeDevice& NodeDevice::operator=(NodeDevice&& nd) noexcept {
    this->~NodeDevice();
    underlying = nd.underlying;
    nd.underlying = nullptr;
    return *this;
}

inline NodeDevice::~NodeDevice() noexcept {
    if (*this)
        virNodeDeviceFree(underlying);
}

constexpr inline NodeDevice::operator bool() const noexcept { return underlying != nullptr; }

inline bool NodeDevice::destroy() noexcept { return virNodeDeviceDestroy(underlying) == 0; }

inline bool NodeDevice::detach(gsl::czstring<> driver_name = nullptr) noexcept { return virNodeDeviceDetachFlags(underlying, driver_name, 0) == 0; }

inline bool NodeDevice::detach() noexcept { return virNodeDeviceDettach(underlying) == 0; }

inline passive<gsl::czstring<>> NodeDevice::getName() const noexcept { return virNodeDeviceGetName(underlying); }

inline passive<gsl::czstring<>> NodeDevice::getParent() const noexcept { return virNodeDeviceGetParent(underlying); }

inline passive<gsl::czstring<>> NodeDevice::getXMLDesc() const noexcept { return virNodeDeviceGetXMLDesc(underlying, 0); }

inline std::string NodeDevice::extractXMLDesc() const noexcept { return {getXMLDesc()}; }

inline auto NodeDevice::listCaps() const noexcept {
    return meta::light::wrap_oparm_owning_fill_static_arr(underlying, virNodeDeviceNumOfCaps, virNodeDeviceListCaps);
}

inline std::vector<std::string> NodeDevice::extractCaps() const {
    return meta::heavy::wrap_oparm_owning_fill_static_arr<std::string>(underlying, virNodeDeviceNumOfCaps, virNodeDeviceListCaps);
}

inline bool NodeDevice::reAttach() noexcept { return virNodeDeviceReAttach(underlying) == 0; }

inline bool NodeDevice::reset() noexcept { return virNodeDeviceReset(underlying) == 0; }

inline NodeDevice NodeDevice::createXML(Connection& conn, gsl::czstring<> xml) noexcept {
    return NodeDevice{virNodeDeviceCreateXML(conn.underlying, xml, 0)};
}
}