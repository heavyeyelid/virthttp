#pragma once

#include <cstring>
#include <exception>
#include <optional>
#include <vector>
#include <gsl/gsl>
#include <libvirt/libvirt.h>
#include "Domain.hpp"
#include "fwd.hpp"
#include "type_ops.hpp"
#include "utility.hpp"

namespace virt {

unsigned long getVersion();

enum class CredentialType : int {
    AUTHNAME = VIR_CRED_AUTHNAME,         // Identify to authorize as
    CNONCE = VIR_CRED_CNONCE,             // Client supplies a nonce
    ECHOPROMPT = VIR_CRED_ECHOPROMPT,     // Challenge response
    EXTERNAL = VIR_CRED_EXTERNAL,         // Externally managed credential More may be
                                          // added - expect the unexpected
    LANGUAGE = VIR_CRED_LANGUAGE,         // RFC 1766 languages, comma separated
    NOECHOPROMPT = VIR_CRED_NOECHOPROMPT, // Challenge response
    PASSPHRASE = VIR_CRED_PASSPHRASE,     // Passphrase secret
    REALM = VIR_CRED_REALM,               // Authentication realm
    USERNAME = VIR_CRED_USERNAME,         // Identity to act as
};

struct ConnectCredential {
    const CredentialType type;
    const gsl::czstring<> prompt;    /* Prompt to show to user */
    const gsl::czstring<> challenge; /* Additional challenge to show */
    const gsl::czstring<> defresult; /* Optional default result */
    std::string result{};

    explicit ConnectCredential(const virConnectCredential& in) noexcept;
};

using ConnectAuthCallback = bool (*)(gsl::span<ConnectCredential>);

template <typename Callback = ConnectAuthCallback> class ConnectionAuth {
  public:
    template <typename Container> inline ConnectionAuth(Container c, Callback callback);

    inline ConnectionAuth(const ConnectionAuth&) = default;

    inline ConnectionAuth(ConnectionAuth&&) noexcept = default;

    explicit operator virConnectAuth() noexcept;

    std::vector<CredentialType> cred_types{};
    Callback callback;
};

class Connection {
    friend Domain;
    friend Network;
    friend NodeDevice;
    friend Stream;

    virConnectPtr underlying = nullptr;

  public:
    enum class Flags : unsigned {
        RO = VIR_CONNECT_RO,                 /* A readonly connection */
        NO_ALIASES = VIR_CONNECT_NO_ALIASES, /* Don't try to resolve URI aliases */
    };
    struct List {
        struct Domains {
            enum class Flag : unsigned {
                DEFAULT = 0,
                ACTIVE = VIR_CONNECT_LIST_DOMAINS_ACTIVE,
                INACTIVE = VIR_CONNECT_LIST_DOMAINS_INACTIVE,

                PERSISTENT = VIR_CONNECT_LIST_DOMAINS_PERSISTENT,
                TRANSIENT = VIR_CONNECT_LIST_DOMAINS_TRANSIENT,

                RUNNING = VIR_CONNECT_LIST_DOMAINS_RUNNING,
                PAUSED = VIR_CONNECT_LIST_DOMAINS_PAUSED,
                SHUTOFF = VIR_CONNECT_LIST_DOMAINS_SHUTOFF,
                OTHER = VIR_CONNECT_LIST_DOMAINS_OTHER,

                MANAGEDSAVE = VIR_CONNECT_LIST_DOMAINS_MANAGEDSAVE,
                NO_MANAGEDSAVE = VIR_CONNECT_LIST_DOMAINS_NO_MANAGEDSAVE,

                AUTOSTART = VIR_CONNECT_LIST_DOMAINS_AUTOSTART,
                NO_AUTOSTART = VIR_CONNECT_LIST_DOMAINS_NO_AUTOSTART,

                HAS_SNAPSHOT = VIR_CONNECT_LIST_DOMAINS_HAS_SNAPSHOT,
                NO_SNAPSHOT = VIR_CONNECT_LIST_DOMAINS_NO_SNAPSHOT,
            };
            class FlagsC : public EnumSetHelper<FlagsC, Flag> {
                using Base = EnumSetHelper<FlagsC, Flag>;
                friend Base;
                constexpr static std::array values = {"active",    "inactive",     "persistent",   "transient",    "running",
                                                      "paused",    "shutoff",      "other",        "managed_save", "no_managed_save",
                                                      "autostart", "no_autostart", "has_snapshot", "no_snapshot"};
            };
            constexpr static FlagsC Flags;
        };
        struct Devices {
            enum class Flags : unsigned {
                DEFAULT = 0,
                SYSTEM = VIR_CONNECT_LIST_NODE_DEVICES_CAP_SYSTEM,
                PCI_DEV = VIR_CONNECT_LIST_NODE_DEVICES_CAP_PCI_DEV,
                USB_DEV = VIR_CONNECT_LIST_NODE_DEVICES_CAP_USB_DEV,
                USB_IFACE = VIR_CONNECT_LIST_NODE_DEVICES_CAP_USB_INTERFACE,
                NETWORK = VIR_CONNECT_LIST_NODE_DEVICES_CAP_NET,
                SCSI_HOST = VIR_CONNECT_LIST_NODE_DEVICES_CAP_SCSI_HOST,
                SCSI_TARGET = VIR_CONNECT_LIST_NODE_DEVICES_CAP_SCSI_TARGET,
                SCSI_DEV = VIR_CONNECT_LIST_NODE_DEVICES_CAP_SCSI,
                STORAGE = VIR_CONNECT_LIST_NODE_DEVICES_CAP_STORAGE,
                FC_HOST = VIR_CONNECT_LIST_NODE_DEVICES_CAP_FC_HOST,
                VPORTS = VIR_CONNECT_LIST_NODE_DEVICES_CAP_VPORTS,
                SCSI_GENERIC = VIR_CONNECT_LIST_NODE_DEVICES_CAP_SCSI_GENERIC,
                DRM = VIR_CONNECT_LIST_NODE_DEVICES_CAP_DRM,
                MEDIATED_CAPABLE = VIR_CONNECT_LIST_NODE_DEVICES_CAP_MDEV_TYPES,
                MEDIATED = VIR_CONNECT_LIST_NODE_DEVICES_CAP_MDEV,
                CCW = VIR_CONNECT_LIST_NODE_DEVICES_CAP_CCW_DEV,
            };
        };
        struct Networks {
            enum class Flag : unsigned {
                DEFAULT = 0,

                INACTIVE = VIR_CONNECT_LIST_NETWORKS_INACTIVE,
                ACTIVE = VIR_CONNECT_LIST_NETWORKS_ACTIVE,

                PERSISTENT = VIR_CONNECT_LIST_NETWORKS_PERSISTENT,
                TRANSIENT = VIR_CONNECT_LIST_NETWORKS_TRANSIENT,

                AUTOSTART = VIR_CONNECT_LIST_NETWORKS_AUTOSTART,
                NO_AUTOSTART = VIR_CONNECT_LIST_NETWORKS_NO_AUTOSTART,
            };
        };
    };
    struct GetAllDomains {
        struct Stats {
            enum class Flags : unsigned {
                ACTIVE = VIR_CONNECT_GET_ALL_DOMAINS_STATS_ACTIVE,
                INACTIVE = VIR_CONNECT_GET_ALL_DOMAINS_STATS_INACTIVE,

                PERSISTENT = VIR_CONNECT_LIST_DOMAINS_PERSISTENT,
                TRANSIENT = VIR_CONNECT_LIST_DOMAINS_TRANSIENT,

                RUNNING = VIR_CONNECT_LIST_DOMAINS_RUNNING,
                PAUSED = VIR_CONNECT_GET_ALL_DOMAINS_STATS_PAUSED,
                SHUTOFF = VIR_CONNECT_LIST_DOMAINS_SHUTOFF,
                OTHER = VIR_CONNECT_GET_ALL_DOMAINS_STATS_OTHER,

                NOWAIT = VIR_CONNECT_GET_ALL_DOMAINS_STATS_NOWAIT,               // report statistics that can be obtained immediately without any
                                                                                 // blocking
                BACKING = VIR_CONNECT_GET_ALL_DOMAINS_STATS_BACKING,             // include backing chain for block stats
                ENFORCE_STATS = VIR_CONNECT_GET_ALL_DOMAINS_STATS_ENFORCE_STATS, // enforce requested stats
            };
        };
    };

  private:
    constexpr explicit Connection(virConnectPtr p) : underlying(p) {}

  public:
    inline explicit Connection(gsl::czstring<> name, bool rd_only = false) noexcept;

    template <typename Callback = virConnectAuthCallbackPtr>
    inline Connection(gsl::czstring<> name, ConnectionAuth<Callback>& auth, Flags flags) noexcept;

    inline Connection(const Connection& conn) noexcept = default;

    constexpr Connection(Connection&& conn) noexcept;

    inline Connection& operator=(const Connection& conn) noexcept = delete;

    inline Connection& operator=(Connection&& conn) noexcept;

    inline ~Connection();

    void ref();

    template <typename Data> void registerCloseCallback(void (*cb)(Data&), std::unique_ptr<Data> data = nullptr);

    void registerCloseCallback(void (*cb)());

    template <typename Data> void unregisterCloseCallback(void (*cb)(Data&));

    void unregisterCloseCallback(void (*cb)());

    void setKeepAlive(int interval, unsigned count);

    [[nodiscard]] inline gsl::zstring<> getCapabilities() const noexcept;

    [[nodiscard]] inline gsl::zstring<> getHostname() const noexcept;

    [[nodiscard]] unsigned long getLibVersion() const;

    [[nodiscard]] int getMaxVcpus(gsl::czstring<> type) const noexcept;

    [[nodiscard]] passive<gsl::zstring<>> getSysInfo(unsigned flags) const noexcept;

    [[nodiscard]] gsl::czstring<> getType() const noexcept;

    [[nodiscard]] passive<gsl::zstring<>> getURI() const noexcept;

    [[nodiscard]] inline bool isAlive() const noexcept;

    [[nodiscard]] inline bool isEncrypted() const noexcept;

    [[nodiscard]] inline bool isSecure() const noexcept;

    bool restore(gsl::czstring<> from) noexcept;

    bool restore(gsl::czstring<> from, gsl::czstring<> dxml, Domain::SaveRestoreFlag flags) noexcept;

    [[nodiscard]] inline int numOfDomains() const noexcept;

    [[nodiscard]] inline int numOfDefinedDomains() const noexcept;

    [[nodiscard]] auto listDomains() const -> std::vector<int>;

    template <typename StrT>[[nodiscard]] auto listDefinedDomains() const = delete;

    [[nodiscard]] auto listAllDomains(List::Domains::Flag flags = List::Domains::Flag::DEFAULT) const -> std::vector<Domain>;

    [[nodiscard]] auto getAllDomainStats(Domain::Stats::Types stats, Connection::GetAllDomains::Stats::Flags flags)
        -> std::vector<Domain::Stats::Record>;

    [[nodiscard]] Domain domainLookupByID(int) const noexcept;

    [[nodiscard]] Domain domainLookupByName(gsl::czstring<>) const noexcept;
    [[nodiscard]] Domain domainLookupByName(const std::string&) const noexcept;

    [[nodiscard]] Domain domainLookupByUUID(gsl::basic_zstring<const unsigned char>) const noexcept;

    [[nodiscard]] Domain domainLookupByUUIDString(gsl::czstring<>) const noexcept;
    [[nodiscard]] Domain domainLookupByUUIDString(const std::string&) const noexcept;

    bool domainSaveImageDefineXML(gsl::czstring<> file, gsl::czstring<> dxml, Domain::SaveRestoreFlag flags) noexcept;

    [[nodiscard]] UniqueZstring domainSaveImageGetXMLDesc(gsl::czstring<> file, Domain::SaveImageXMLFlag flags) const noexcept;

    [[nodiscard]] virNodeInfo nodeGetInfo() const;

    [[nodiscard]] unsigned long long nodeGetFreeMemory() const;

    [[nodiscard]] std::vector<unsigned long long> nodeGetCellsFreeMemory() const;

    constexpr explicit operator bool() const noexcept { return underlying != nullptr; }

    [[nodiscard]] Network networkLookupByUUID(gsl::basic_zstring<const unsigned char> uuid) const noexcept;

    [[nodiscard]] Network networkLookupByName(gsl::czstring<> name) const noexcept;
    [[nodiscard]] Network networkLookupByName(const std::string& name) const noexcept;
    [[nodiscard]] Network networkLookupByUUIDString(gsl::czstring<> uuid_str) const noexcept;
    [[nodiscard]] Network networkLookupByUUIDString(const std::string& uuid_str) const noexcept;
    /**
     * (Light) List all networks
     * @param active
     * @param persistent
     * @param autostart
     * @return std:unique_ptr<Network[], ?>: A wrapped array of networks extended with begin() and end(), or {nullptr} in case of error
     * */
    [[nodiscard]] auto listAllNetworks(List::Networks::Flag) const noexcept;
    [[nodiscard]] inline auto listDefinedNetworksNames() const noexcept;
    [[nodiscard]] inline auto listNetworksNames() const noexcept;
    [[nodiscard]] std::vector<Network> extractAllNetworks(List::Networks::Flag) const;
    [[nodiscard]] std::vector<std::string> extractDefinedNetworksNames() const;
    [[nodiscard]] std::vector<std::string> extractNetworksNames() const;

    [[nodiscard]] auto listAllDevices(List::Devices::Flags flags = List::Devices::Flags::DEFAULT) const noexcept;
    [[nodiscard]] std::vector<NodeDevice> extractAllDevices(List::Devices::Flags flags = List::Devices::Flags::DEFAULT) const;

    [[nodiscard]] auto listDevicesNames(const std::string& capability) const noexcept;
    [[nodiscard]] std::vector<std::string> extractDevicesNames(const std::string& capability) const;

    [[nodiscard]] NodeDevice deviceLookupByName(gsl::czstring<> name) const noexcept;

    [[nodiscard]] NodeDevice deviceLookupSCSIHostByWWN(gsl::czstring<> wwnn, gsl::czstring<> wwpn) const noexcept;
};

[[nodiscard]] constexpr inline Connection::Flags operator|(Connection::Flags lhs, Connection::Flags rhs) noexcept;

[[nodiscard]] constexpr inline Connection::List::Domains::Flag operator|(Connection::List::Domains::Flag lhs,
                                                                         Connection::List::Domains::Flag rhs) noexcept;
constexpr inline Connection::List::Domains::Flag& operator|=(Connection::List::Domains::Flag& lhs, Connection::List::Domains::Flag rhs) noexcept;
[[nodiscard]] constexpr inline Connection::List::Networks::Flag operator|(Connection::List::Networks::Flag lhs,
                                                                          Connection::List::Networks::Flag rhs) noexcept;
constexpr inline Connection::List::Networks::Flag& operator|=(Connection::List::Networks::Flag& lhs, Connection::List::Networks::Flag rhs) noexcept;

[[nodiscard]] constexpr inline Connection::GetAllDomains::Stats::Flags operator|(Connection::GetAllDomains::Stats::Flags lhs,
                                                                                 Connection::GetAllDomains::Stats::Flags rhs) noexcept;

constexpr inline Connection::List::Devices::Flags operator|(Connection::List::Devices::Flags lhs, Connection::List::Devices::Flags rhs) noexcept;
}