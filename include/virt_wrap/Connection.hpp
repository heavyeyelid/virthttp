#ifndef VIRTPP_CONNECTION_HPP
#define VIRTPP_CONNECTION_HPP

#include <cstring>
#include <exception>
#include <optional>
#include <vector>
#include <gsl/gsl>
#include <libvirt/libvirt.h>
#include "Domain.hpp"
#include "fwd.hpp"
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

    inline explicit ConnectCredential(const virConnectCredential& in) noexcept;
};

using ConnectAuthCallback = bool (*)(gsl::span<ConnectCredential>);

template <typename Callback = ConnectAuthCallback> class ConnectionAuth {
  public:
    template <typename Container> inline ConnectionAuth(Container c, Callback callback);

    inline ConnectionAuth(const ConnectionAuth&) = default;

    inline ConnectionAuth(ConnectionAuth&&) noexcept = default;

    inline explicit operator virConnectAuth() noexcept;

    std::vector<CredentialType> cred_types{};
    Callback callback;
};

class Connection {
    friend Domain;
    friend Network;
    friend NodeDevice;
    friend StoragePool;
    friend StorageVol;
    friend Stream;

    virConnectPtr underlying = nullptr;

  public:
    enum class Flags : unsigned {
        RO = VIR_CONNECT_RO,                 /* A readonly connection */
        NO_ALIASES = VIR_CONNECT_NO_ALIASES, /* Don't try to resolve URI aliases */
    };
    struct List {
        struct Domains {
            class Flag;
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
        struct StoragePool {
            class Flag;
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

    inline void ref();

    template <typename Data> void registerCloseCallback(void (*cb)(Data&), std::unique_ptr<Data> data = nullptr);

    inline void registerCloseCallback(void (*cb)());

    template <typename Data> void unregisterCloseCallback(void (*cb)(Data&));

    inline void unregisterCloseCallback(void (*cb)());

    inline void setKeepAlive(int interval, unsigned count);

    [[nodiscard]] inline gsl::zstring<> findStoragePoolSources(gsl::czstring<> type, gsl::czstring<>) const noexcept;
#if LIBVIR_VERSION_NUMBER >= 5002000
    [[nodiscard]] inline gsl::zstring<> getStoragePoolCapabilities() const noexcept;
#endif

    [[nodiscard]] inline gsl::zstring<> getCapabilities() const noexcept;

    [[nodiscard]] inline gsl::zstring<> getHostname() const noexcept;

    [[nodiscard]] inline unsigned long getLibVersion() const;

    [[nodiscard]] inline int getMaxVcpus(gsl::czstring<> type) const noexcept;

    [[nodiscard]] inline passive<gsl::zstring<>> getSysInfo(unsigned flags) const noexcept;

    [[nodiscard]] inline gsl::czstring<> getType() const noexcept;

    [[nodiscard]] inline passive<gsl::zstring<>> getURI() const noexcept;

    [[nodiscard]] inline bool isAlive() const noexcept;

    [[nodiscard]] inline bool isEncrypted() const noexcept;

    [[nodiscard]] inline bool isSecure() const noexcept;

    inline bool restore(gsl::czstring<> from) noexcept;

    inline bool restore(gsl::czstring<> from, gsl::czstring<> dxml, Domain::SaveRestoreFlag flags) noexcept;

    [[nodiscard]] inline int numOfDomains() const noexcept;

    [[nodiscard]] inline int numOfDefinedDomains() const noexcept;

    [[nodiscard]] inline auto listDomains() const -> std::vector<int>;

    template <typename StrT>[[nodiscard]] auto listDefinedDomains() const = delete;

    [[nodiscard]] inline auto listAllDomains(List::Domains::Flag flags) const -> std::vector<Domain>;

    [[nodiscard]] inline auto getAllDomainStats(Domain::Stats::Types stats, Connection::GetAllDomains::Stats::Flags flags)
        -> std::vector<Domain::Stats::Record>;

    [[nodiscard]] inline Domain domainLookupByID(int) const noexcept;

    [[nodiscard]] inline Domain domainLookupByName(gsl::czstring<>) const noexcept;
    [[nodiscard]] inline Domain domainLookupByName(const std::string&) const noexcept;

    [[nodiscard]] inline Domain domainLookupByUUID(gsl::basic_zstring<const unsigned char>) const noexcept;

    [[nodiscard]] inline Domain domainLookupByUUIDString(gsl::czstring<>) const noexcept;
    [[nodiscard]] inline Domain domainLookupByUUIDString(const std::string&) const noexcept;

    inline bool domainSaveImageDefineXML(gsl::czstring<> file, gsl::czstring<> dxml, Domain::SaveRestoreFlag flags) noexcept;

    [[nodiscard]] inline UniqueZstring domainSaveImageGetXMLDesc(gsl::czstring<> file, Domain::SaveImageXMLFlag flags) const noexcept;

    [[nodiscard]] inline virNodeInfo nodeGetInfo() const;

    [[nodiscard]] inline unsigned long long nodeGetFreeMemory() const;

    [[nodiscard]] inline std::vector<unsigned long long> nodeGetCellsFreeMemory() const;

    constexpr explicit operator bool() const noexcept { return underlying != nullptr; }

    [[nodiscard]] inline Network networkLookupByUUID(gsl::basic_zstring<const unsigned char> uuid) const noexcept;

    [[nodiscard]] inline Network networkLookupByName(gsl::czstring<> name) const noexcept;
    [[nodiscard]] inline Network networkLookupByName(const std::string& name) const noexcept;
    [[nodiscard]] inline Network networkLookupByUUIDString(gsl::czstring<> uuid_str) const noexcept;
    [[nodiscard]] inline Network networkLookupByUUIDString(const std::string& uuid_str) const noexcept;
    /**
     * (Light) List all networks
     * @param active
     * @param persistent
     * @param autostart
     * @return std:unique_ptr<Network[], ?>: A wrapped array of networks extended with begin() and end(), or {nullptr} in case of error
     * */
    [[nodiscard]] inline auto listAllNetworks(List::Networks::Flag) const noexcept;
    [[nodiscard]] inline auto listDefinedNetworksNames() const noexcept;
    [[nodiscard]] inline auto listNetworksNames() const noexcept;
    [[nodiscard]] inline std::vector<Network> extractAllNetworks(List::Networks::Flag) const;
    [[nodiscard]] inline std::vector<std::string> extractDefinedNetworksNames() const;
    [[nodiscard]] inline std::vector<std::string> extractNetworksNames() const;

    [[nodiscard]] inline auto listAllDevices(List::Devices::Flags flags = List::Devices::Flags::DEFAULT) const noexcept;
    [[nodiscard]] inline std::vector<NodeDevice> extractAllDevices(List::Devices::Flags flags = List::Devices::Flags::DEFAULT) const;

    [[nodiscard]] inline auto listDevicesNames(const std::string& capability) const noexcept;
    [[nodiscard]] inline std::vector<std::string> extractDevicesNames(const std::string& capability) const;

    [[nodiscard]] inline NodeDevice deviceLookupByName(gsl::czstring<> name) const noexcept;

    [[nodiscard]] inline NodeDevice deviceLookupSCSIHostByWWN(gsl::czstring<> wwnn, gsl::czstring<> wwpn) const noexcept;

    [[nodiscard]] inline auto listAllStoragePools(List::StoragePool::Flag flags) const noexcept;
    [[nodiscard]] inline std::vector<StoragePool> extractAllStoragePools(List::StoragePool::Flag) const;
    [[nodiscard]] inline auto listDefinedStoragePoolsNames() const noexcept;
    [[nodiscard]] inline auto listStoragePoolsNames() const noexcept;
    [[nodiscard]] inline int numOfDefinedStoragePools() const noexcept;
    [[nodiscard]] inline int numOfStoragePools() const noexcept;

    [[nodiscard]] inline StoragePool storagePoolLookupByName(gsl::czstring<> name) const noexcept;
    [[nodiscard]] inline StoragePool storagePoolLookupByTargetPath(gsl::czstring<> path) const noexcept;
    [[nodiscard]] inline StoragePool storagePoolLookupByUUID(gsl::basic_zstring<const unsigned char> uuid) const noexcept;
    [[nodiscard]] inline StoragePool storagePoolLookupByUUIDString(gsl::czstring<> uuidstr) const noexcept;

    [[nodiscard]] inline StorageVol storageVolLookupByKey(gsl::czstring<> key) const noexcept;
    [[nodiscard]] inline StorageVol storageVolLookupByPath(gsl::czstring<> path) const noexcept;
};

} // namespace virt
#include "enums/Connection/Connection.hpp"
namespace virt {

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
} // namespace virt

#include "impl/Connection.hpp"

#endif