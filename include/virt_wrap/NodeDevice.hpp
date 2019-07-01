#pragma once

#include <string>
#include <libvirt/libvirt.h>

namespace tmp {
int virConnectListAllNodeDevices(virConnectPtr conn, virNodeDevicePtr** devices, unsigned int flags);
/*
int	virConnectNodeDeviceEventDeregisterAny	(virConnectPtr conn,
                                                  int callbackID);
typedef virConnectNodeDeviceEventGenericCallback
void	virConnectNodeDeviceEventGenericCallback	(virConnectPtr conn,
                                                     virNodeDevicePtr dev,
                                                     void * opaque);

typedef virConnectNodeDeviceEventLifecycleCallback
void	virConnectNodeDeviceEventLifecycleCallback	(virConnectPtr conn,
                                                       virNodeDevicePtr dev,
                                                       int event,
                                                       int detail,
                                                       void * opaque);

int	virConnectNodeDeviceEventRegisterAny	(virConnectPtr conn,
                                                virNodeDevicePtr dev,
                                                int eventID,
                                                virConnectNodeDeviceEventGenericCallback cb,
                                                void * opaque,
                                                virFreeCallback freecb);
                                                */

virNodeDevicePtr virNodeDeviceLookupByName(virConnectPtr conn, const char* name);
virNodeDevicePtr virNodeDeviceLookupSCSIHostByWWN(virConnectPtr conn, const char* wwnn, const char* wwpn, unsigned int flags);

int virNodeListDevices(virConnectPtr conn, const char* cap, char** const names, int maxnames, unsigned int flags);
int virNodeNumOfDevices(virConnectPtr conn, const char* cap, unsigned int flags);
} // namespace tmp

namespace virt {
class NodeDevice {
    friend Connection;

    virNodeDevicePtr underlying = nullptr;

  public:
    constexpr inline explicit NodeDevice(virNodeDevicePtr ptr = nullptr) noexcept;

    NodeDevice(const NodeDevice&) = delete;

    constexpr inline NodeDevice(NodeDevice&&) noexcept;

    NodeDevice& operator=(const NodeDevice&) = delete;

    inline NodeDevice& operator=(NodeDevice&&) noexcept;

    inline ~NodeDevice() noexcept;

    constexpr inline explicit operator bool() const noexcept;

    bool destroy() noexcept;

    bool detach(gsl::czstring<> driver_name_or_null) noexcept;

    bool detach() noexcept;

    [[nodiscard]] passive<gsl::czstring<>> getName() const noexcept;

    [[nodiscard]] passive<gsl::czstring<>> getParent() const noexcept;

    [[nodiscard]] passive<gsl::czstring<>> getXMLDesc() const noexcept;

    [[nodiscard]] std::string extractXMLDesc() const noexcept;

    [[nodiscard]] auto listCaps() const noexcept;

    [[nodiscard]] std::vector<std::string> extractCaps() const;

    bool reAttach() noexcept;

    bool reset() noexcept;

    [[nodiscard]] static NodeDevice createXML(Connection& conn, gsl::czstring<> xml) noexcept;
};
}