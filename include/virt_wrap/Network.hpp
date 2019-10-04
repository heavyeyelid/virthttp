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

  public:
    constexpr Network() noexcept = default;
    Network(const Network&) = delete;
    constexpr Network(Network&&) noexcept;
    inline ~Network() noexcept;
    Network& operator=(const Network&) = delete;
    inline Network& operator=(Network&&) noexcept;
    constexpr explicit inline Network(virNetworkPtr p) : underlying(p) {}

    constexpr explicit operator bool() const noexcept { return underlying != nullptr; }
    [[nodiscard]] inline auto getBridgeName() const noexcept;

    [[nodiscard]] inline Connection getConnect() const noexcept;

    [[nodiscard]] inline passive<const char*> getName() const noexcept;

    [[nodiscard]] inline std::string extractName() const noexcept;

    [[nodiscard]] inline auto getUUID() const noexcept;

    [[nodiscard]] inline auto getUUIDString() const noexcept -> std::optional<std::array<char, VIR_UUID_STRING_BUFLEN>>;

    [[nodiscard]] inline auto extractUUIDString() const -> std::string;

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

    inline bool destroy() noexcept;
    inline bool undefine() noexcept;

    static inline Network createXML(Connection& conn, gsl::czstring<> xml);
    static inline Network defineXML(Connection& conn, gsl::czstring<> xml);
};
} // namespace virt

#include "impl/Network.hpp"