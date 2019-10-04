#pragma once

#include <string>
#include <libvirt/libvirt.h>

namespace tmp {
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

} // namespace tmp

namespace virt {
class NodeDevice {
    friend Connection;

    virNodeDevicePtr underlying = nullptr;

  public:
    constexpr inline explicit NodeDevice(virNodeDevicePtr ptr = nullptr) noexcept;

    NodeDevice(const NodeDevice&) = delete;

    constexpr NodeDevice(NodeDevice&&) noexcept;

    NodeDevice& operator=(const NodeDevice&) = delete;

    inline NodeDevice& operator=(NodeDevice&&) noexcept;

    inline ~NodeDevice() noexcept;

    constexpr explicit operator bool() const noexcept;

    inline bool destroy() noexcept;

    inline bool detach(gsl::czstring<> driver_name_or_null) noexcept;

    inline bool detach() noexcept;

    [[nodiscard]] inline passive<gsl::czstring<>> getName() const noexcept;

    [[nodiscard]] inline passive<gsl::czstring<>> getParent() const noexcept;

    [[nodiscard]] inline passive<gsl::czstring<>> getXMLDesc() const noexcept;

    [[nodiscard]] inline std::string extractXMLDesc() const noexcept;

    [[nodiscard]] inline auto listCaps() const noexcept;

    [[nodiscard]] inline std::vector<std::string> extractCaps() const;

    inline bool reAttach() noexcept;

    inline bool reset() noexcept;

    [[nodiscard]] inline static NodeDevice createXML(Connection& conn, gsl::czstring<> xml) noexcept;
};
} // namespace virt

#include "impl/NodeDevice.hpp"