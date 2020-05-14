#ifndef VIRTPP_CONNECTION_HPP
#define VIRTPP_CONNECTION_HPP

#include <cstring>
#include <exception>
#include <optional>
#include <vector>
#include <gsl/gsl>
#include <libvirt/libvirt.h>
#include "enums/Connection/Decls.hpp"
#include "enums/Domain/Decls.hpp"
#include "enums/Domain/SaveRestoreFlag.hpp"
#include "fwd.hpp"
#include "utility.hpp"

namespace virt {

unsigned long getVersion();

struct ConnectCredential {
    const enums::connection::CredentialType type;
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

    std::vector<enums::connection::CredentialType> cred_types{};
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
    struct List {
        struct Domains {
            class Flag;
        };
        struct StoragePool {
            class Flag;
        };
    };

  private:
    constexpr explicit Connection(virConnectPtr p) : underlying(p) {}

  public:
    inline explicit Connection(gsl::czstring<> name, bool rd_only = false) noexcept;

    template <typename Callback = virConnectAuthCallbackPtr>
    inline Connection(gsl::czstring<> name, ConnectionAuth<Callback>& auth, enums::connection::Flags flags) noexcept;

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

    [[nodiscard]] inline auto getAllDomainStats(enums::domain::stats::Types stats, enums::connection::get_all_domains::stats::Flags flags)
        -> std::vector<enums::domain::stats::Record>;

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