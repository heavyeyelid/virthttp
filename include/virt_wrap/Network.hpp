#pragma once
#include <optional>
#include <string>
#include <vector>
#include <gsl/gsl>
#include <libvirt/libvirt.h>
#include "fwd.hpp"
#include "tfe.hpp"
#include "utility.hpp"

namespace virt {

class Network {
    friend Connection;
    enum class XMLFlags : unsigned {
        DEFAULT = 0,
        INACTIVE = VIR_NETWORK_XML_INACTIVE,
    };

  private:
    virNetworkPtr underlying = nullptr;

    explicit Network(virNetworkPtr p) : underlying(p) {} // 2Move

  public:
    constexpr Network() noexcept = default;
    Network(const Network&) = delete;
    Network(Network&&) noexcept = default;
    inline ~Network() noexcept { // 2Move
        if (*this)
            virNetworkFree(underlying);
    }
    Network& operator=(const Network&) = delete;
    Network& operator=(Network&&) noexcept = default;

    constexpr explicit operator bool() const noexcept { return underlying != nullptr; }
    [[nodiscard]] inline auto getBridgeName() const noexcept;

    [[nodiscard]] inline Connection getConnect() const noexcept;

    [[nodiscard]] inline passive<const char*> getName() const noexcept;

    [[nodiscard]] inline std::string extractName() const noexcept;

    [[nodiscard]] inline auto getUUID() const noexcept;

    [[nodiscard]] inline auto getUUIDString() const noexcept;

    [[nodiscard]] inline auto getXMLDesc(XMLFlags flags = XMLFlags::DEFAULT) const noexcept;

    [[nodiscard]] inline std::string extractXMLDesc(XMLFlags flags = XMLFlags::DEFAULT) const noexcept;

    [[nodiscard]] inline TFE isActive() const noexcept;

    [[nodiscard]] inline TFE isPersistent() const noexcept;

    [[nodiscard]] inline auto getDHCPLeases(gsl::czstring<> mac) const noexcept;

    [[nodiscard]] inline auto extractDHCPLeases(gsl::czstring<> mac) const -> std::optional<std::vector<virNetworkDHCPLease>>;

    inline bool setAutostart(bool autostart) noexcept;
    [[nodiscard]] inline TFE getAutostart() const noexcept;

    inline bool create() noexcept;
    // inline bool update(...) noexcept;

    bool destroy() noexcept;
    bool undefine() noexcept;

    static inline Network createXML(Connection& conn, gsl::czstring<> xml);
    static inline Network defineXML(Connection& conn, gsl::czstring<> xml);
};
}