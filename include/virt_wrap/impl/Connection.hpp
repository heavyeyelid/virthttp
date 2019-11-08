//
// Created by _as on 2019-02-01.
//

#pragma once

#include <cstring>
#include <exception>
#include <vector>
#include <gsl/gsl>
#include <libvirt/libvirt.h>
#include "../Connection.hpp"
#include "../Domain.hpp"
#include "../Network.hpp"
#include "../NodeDevice.hpp"
#include "../StoragePool.hpp"
#include "../fwd.hpp"
#include "../utility.hpp"

namespace virt {

unsigned long getVersion() {
    unsigned long ret{};
    if (virGetVersion(&ret, nullptr, nullptr))
        throw std::runtime_error{"virGetVersion"};
    return ret;
}

ConnectCredential::ConnectCredential(const virConnectCredential& in) noexcept
    : type(CredentialType(in.type)), prompt(in.prompt), challenge(in.challenge), defresult(in.defresult) {}

template <typename Callback>
template <typename Container>
inline ConnectionAuth<Callback>::ConnectionAuth(Container c, Callback callback) : callback(callback) {
    if constexpr (std::is_same_v<Container, std::vector<CredentialType>>)
        cred_types = std::forward(c);
    else {
        cred_types.reserve(c.size());
        std::copy(c.begin(), c.end(), std::back_inserter(cred_types));
    }
}

template <typename Callback> ConnectionAuth<Callback>::operator virConnectAuth() noexcept {
    virConnectAuth ret;
    ret.credtype = reinterpret_cast<int*>(cred_types.data());
    ret.ncredtype = gsl::narrow_cast<unsigned>(cred_types.size());
    ret.cbdata = reinterpret_cast<void*>(&callback);

    ret.cb = +[](virConnectCredentialPtr creds_ptr, unsigned int ncreds, void* cbdata) -> int {
        std::vector<ConnectCredential> creds{};
        creds.reserve(ncreds);
        std::transform(creds_ptr, creds_ptr + ncreds, std::back_inserter(creds), [](const auto& cred) { return ConnectCredential{cred}; });
        const bool ret = (*reinterpret_cast<Callback*>(cbdata))(creds);
        auto it = creds_ptr;
        for (const auto& [t, p, c, d, res] : creds) {
            it->result = static_cast<char*>(malloc(it->resultlen = res.length() + 1u));
            std::copy(res.begin(), res.end(), it->result);
            ++it;
        }
        return ret;
    };

    return ret;
}

inline Connection::Connection(gsl::czstring<> name, bool rd_only) noexcept {
    underlying = rd_only ? virConnectOpenReadOnly(name) : virConnectOpen(name);
}

template <typename Callback> inline Connection::Connection(gsl::czstring<> name, ConnectionAuth<Callback>& auth, Flags flags) noexcept {
    virConnectAuth c_auth = auth;
    underlying = virConnectOpenAuth(name, &c_auth, to_integral(flags));
}

constexpr Connection::Connection(Connection&& conn) noexcept : underlying(conn.underlying) { conn.underlying = nullptr; }

inline Connection& Connection::operator=(Connection&& conn) noexcept {
    this->~Connection();
    underlying = conn.underlying;
    conn.underlying = nullptr;
    return *this;
}

inline Connection::~Connection() {
    if (underlying)
        virConnectClose(underlying);
}

void Connection::ref() {
    if (virConnectRef(underlying))
        throw std::runtime_error{"virConnectRef"};
}

template <typename Data> void Connection::registerCloseCallback(void (*cb)(Data&), std::unique_ptr<Data> data) {
    virConnectRegisterCloseCallback(underlying, reinterpret_cast<virConnectCloseFunc>(cb), data.release(), &data.get_deleter());
}

void Connection::registerCloseCallback(void (*cb)()) {
    if (virConnectRegisterCloseCallback(underlying, reinterpret_cast<virConnectCloseFunc>(cb), nullptr, nullptr))
        throw std::runtime_error{"virConnectRegisterCloseCallback"};
}

template <typename Data> void Connection::unregisterCloseCallback(void (*cb)(Data&)) {
    if (virConnectUnregisterCloseCallback(underlying, reinterpret_cast<virConnectCloseFunc>(cb)))
        throw std::runtime_error{"unregisterCloseCallback"};
}

void Connection::unregisterCloseCallback(void (*cb)()) {
    if (virConnectUnregisterCloseCallback(underlying, reinterpret_cast<virConnectCloseFunc>(cb)))
        throw std::runtime_error{"unregisterCloseCallback"};
}

void Connection::setKeepAlive(int interval, unsigned count) {
    if (virConnectSetKeepAlive(underlying, interval, count))
        throw std::runtime_error{"virConnectSetKeepAlive"};
}

[[nodiscard]] inline gsl::zstring<> Connection::findStoragePoolSources(gsl::czstring<> type, gsl::czstring<> srcSpec) const noexcept {
    return virConnectFindStoragePoolSources(underlying, type, srcSpec, 0u);
}
#if LIBVIR_VERSION_NUMBER >= 5002000
[[nodiscard]] inline gsl::zstring<> Connection::getStoragePoolCapabilities() const noexcept {
    return virConnectGetStoragePoolCapabilities(underlying, 0u);
}
#endif

inline gsl::zstring<> Connection::getCapabilities() const noexcept { return virConnectGetCapabilities(underlying); }

inline gsl::zstring<> Connection::getHostname() const noexcept { return virConnectGetHostname(underlying); }

unsigned long Connection::getLibVersion() const {
    unsigned long ret{};
    if (virConnectGetLibVersion(underlying, &ret))
        throw std::runtime_error{"virConnectGetLibVersion"};
    return ret;
}

int Connection::getMaxVcpus(gsl::czstring<> type) const noexcept { return virConnectGetMaxVcpus(underlying, type); }

passive<gsl::zstring<>> Connection::getSysInfo(unsigned flags) const noexcept { return virConnectGetSysinfo(underlying, flags); }

gsl::czstring<> Connection::getType() const noexcept { return virConnectGetType(underlying); }

passive<gsl::zstring<>> Connection::getURI() const noexcept { return virConnectGetURI(underlying); }

inline bool Connection::isAlive() const noexcept { return virConnectIsAlive(underlying) > 0; }

inline bool Connection::isEncrypted() const noexcept { return virConnectIsEncrypted(underlying) > 0; }

inline bool Connection::isSecure() const noexcept { return virConnectIsSecure(underlying) > 0; }

inline int Connection::numOfDomains() const noexcept { return virConnectNumOfDomains(underlying); }

inline int Connection::numOfDefinedDomains() const noexcept { return virConnectNumOfDefinedDomains(underlying); }

auto Connection::listDomains() const -> std::vector<int> {
    std::vector<int> ret{};
    const auto max_size = numOfDomains();
    ret.resize(gsl::narrow_cast<unsigned>(max_size));
    const auto res = virConnectListDomains(underlying, ret.data(), max_size);
    if (res < 0)
        throw std::runtime_error{"virConnectListDomains"};
    ret.resize(gsl::narrow_cast<unsigned>(res));
    return ret;
}

template <> auto Connection::listDefinedDomains<gsl::zstring<>>() const {
    std::vector<gsl::owner<gsl::zstring<>>> buf{};
    const auto max_size = numOfDomains();
    buf.resize(gsl::narrow_cast<unsigned>(max_size));
    const auto res = virConnectListDefinedDomains(underlying, buf.data(), max_size);
    if (res < 0)
        throw std::runtime_error{"virConnectListDefinedDomains"};
    constexpr auto d = +[](gsl::owner<char*> p) { std::free(p); };
    using D = decltype(d);
    std::vector<std::unique_ptr<char[], D>> ret;
    ret.reserve(gsl::narrow_cast<unsigned>(res));
    std::transform(std::make_move_iterator(buf.begin()), std::make_move_iterator(buf.end()), std::back_inserter(ret),
                   [=](gsl::owner<gsl::zstring<>> str) { return std::unique_ptr<char[], D>(str, d); });
    return ret;
}

template <> auto Connection::listDefinedDomains<std::string>() const {
    std::vector<gsl::owner<gsl::zstring<>>> buf{};
    const auto max_size = numOfDefinedDomains();
    buf.resize(gsl::narrow_cast<unsigned>(max_size));
    const auto res = virConnectListDefinedDomains(underlying, buf.data(), max_size);
    if (res < 0)
        throw std::runtime_error{"virConnectListDefinedDomains"};
    std::vector<std::string> ret;
    ret.reserve(gsl::narrow_cast<unsigned>(res));
    std::transform(std::make_move_iterator(buf.begin()), std::make_move_iterator(buf.end()), std::back_inserter(ret),
                   [](gsl::owner<gsl::zstring<>> str) {
                       const std::string ret{str};
                       std::free(str);
                       return ret;
                   });
    return ret;
}

auto Connection::listAllDomains(List::Domains::Flag flags) const -> std::vector<Domain> {
    std::vector<Domain> ret;
    gsl::owner<virDomainPtr*> domains;

    const auto res = virConnectListAllDomains(underlying, &domains, to_integral(flags));
    if (res < 0)
        throw std::runtime_error{"virConnectListAllDomains"};

    ret.reserve(static_cast<unsigned>(res));
    std::for_each(domains, domains + res, [&](virDomainPtr d) { ret.emplace_back(d); });
    std::free(domains);

    return ret;
}

auto Connection::getAllDomainStats(Domain::Stats::Types stats, Connection::GetAllDomains::Stats::Flags flags) -> std::vector<Domain::Stats::Record> {
    virDomainStatsRecordPtr* ptr;
    const int res = virConnectGetAllDomainStats(underlying, to_integral(stats), &ptr, to_integral(flags));
    if (res < 0)
        throw std::runtime_error{"virConnectGetAllDomainStats"};

    std::vector<Domain::Stats::Record> recs;
    recs.reserve(static_cast<std::size_t>(res));
    std::transform(*ptr, *ptr + res, std::back_inserter(recs), [](const virDomainStatsRecord& rec) { return Domain::Stats::Record{rec}; });
    virDomainStatsRecordListFree(ptr);
    return recs;
}

Domain Connection::domainLookupByID(int id) const noexcept { return Domain{virDomainLookupByID(underlying, id)}; }

Domain Connection::domainLookupByName(gsl::czstring<> name) const noexcept { return Domain{virDomainLookupByName(underlying, name)}; }
Domain Connection::domainLookupByName(const std::string& name) const noexcept { return domainLookupByName(name.c_str()); }

Domain Connection::domainLookupByUUID(gsl::basic_zstring<const unsigned char> uuid) const noexcept {
    return Domain{virDomainLookupByUUID(underlying, uuid)};
}

Domain Connection::domainLookupByUUIDString(gsl::czstring<> uuid_str) const noexcept {
    return Domain{virDomainLookupByUUIDString(underlying, uuid_str)};
}
Domain Connection::domainLookupByUUIDString(const std::string& uuid_str) const noexcept { return domainLookupByUUIDString(uuid_str.c_str()); }

bool Connection::domainSaveImageDefineXML(gsl::czstring<> file, gsl::czstring<> dxml, Domain::SaveRestoreFlag flags) noexcept {
    return virDomainSaveImageDefineXML(underlying, file, dxml, to_integral(flags)) >= 0;
}

[[nodiscard]] UniqueZstring Connection::domainSaveImageGetXMLDesc(gsl::czstring<> file, Domain::SaveImageXMLFlag flags) const noexcept {
    return UniqueZstring{virDomainSaveImageGetXMLDesc(underlying, file, to_integral(flags))};
}

Network Connection::networkLookupByUUID(gsl::basic_zstring<const unsigned char> uuid) const noexcept {
    return Network{virNetworkLookupByUUID(underlying, uuid)};
}

Network Connection::networkLookupByName(gsl::czstring<> name) const noexcept { return Network{virNetworkLookupByName(underlying, name)}; }
Network Connection::networkLookupByName(const std::string& name) const noexcept { return networkLookupByName(name.c_str()); }
Network Connection::networkLookupByUUIDString(gsl::czstring<> uuid_str) const noexcept {
    return Network{virNetworkLookupByUUIDString(underlying, uuid_str)};
}
Network Connection::networkLookupByUUIDString(const std::string& uuid_str) const noexcept { return networkLookupByUUIDString(uuid_str.c_str()); }

virNodeInfo Connection::nodeGetInfo() const {
    virNodeInfo ret{};
    virNodeGetInfo(underlying, &ret);
    return ret;
}

unsigned long long Connection::nodeGetFreeMemory() const { return virNodeGetFreeMemory(underlying); }

std::vector<unsigned long long> Connection::nodeGetCellsFreeMemory() const {
    std::vector<unsigned long long> ret{};
    ret.resize(nodeGetInfo().nodes);
    const int res = virNodeGetCellsFreeMemory(underlying, ret.data(), 0, gsl::narrow_cast<int>(ret.size()));
    if (res < 0)
        throw std::runtime_error{"virNodeGetCellsFreeMemory"};
    return ret;
}

[[nodiscard]] auto Connection::listAllNetworks(Connection::List::Networks::Flag flags) const noexcept {
    return virt::meta::light::wrap_opram_owning_set_destroyable_arr<Network>(underlying, virConnectListAllNetworks, to_integral(flags));
}

inline auto Connection::listDefinedNetworksNames() const noexcept {
    return virt::meta::light::wrap_oparm_owning_fill_freeable_arr(underlying, virConnectNumOfDefinedNetworks, virConnectListDefinedNetworks);
}

auto Connection::listNetworksNames() const noexcept {
    return virt::meta::light::wrap_oparm_owning_fill_freeable_arr(underlying, virConnectNumOfNetworks, virConnectListNetworks);
}

auto Connection::extractDefinedNetworksNames() const -> std::vector<std::string> {
    return virt::meta::heavy::wrap_oparm_owning_fill_freeable_arr<std::string>(underlying, virConnectNumOfDefinedNetworks,
                                                                               virConnectListDefinedNetworks);
}

auto Connection::extractNetworksNames() const -> std::vector<std::string> {
    return virt::meta::heavy::wrap_oparm_owning_fill_freeable_arr<std::string>(underlying, virConnectNumOfNetworks, virConnectListNetworks);
}

std::vector<Network> Connection::extractAllNetworks(List::Networks::Flag flags) const {
    return virt::meta::heavy::wrap_opram_owning_set_destroyable_arr<Network>(underlying, virConnectListAllNetworks, to_integral(flags));
}

auto Connection::listAllDevices(List::Devices::Flags flags) const noexcept {
    return meta::light::wrap_opram_owning_set_destroyable_arr<NodeDevice, UniqueNullTerminatedSpan>(underlying, virConnectListAllNodeDevices,
                                                                                                    to_integral(flags));
}
std::vector<NodeDevice> Connection::extractAllDevices(List::Devices::Flags flags) const {
    return meta::heavy::wrap_opram_owning_set_destroyable_arr<NodeDevice>(underlying, virConnectListAllNodeDevices, to_integral(flags));
}

auto Connection::listDevicesNames(const std::string& capability) const noexcept {
    return meta::light::wrap_oparm_owning_fill_freeable_arr(
        underlying, [=](decltype(underlying) u) { return virNodeNumOfDevices(u, capability.data(), 0); },
        [=](decltype(underlying) u, char** ptr, int max) { return virNodeListDevices(u, capability.data(), ptr, max, 0); });
}

std::vector<std::string> Connection::extractDevicesNames(const std::string& capability) const {
    return meta::heavy::wrap_oparm_owning_fill_freeable_arr<std::string>(
        underlying, [=](decltype(underlying) u) { return virNodeNumOfDevices(u, capability.data(), 0); },
        [=](decltype(underlying) u, char** ptr, int max) { return virNodeListDevices(u, capability.data(), ptr, max, 0); });
}

NodeDevice Connection::deviceLookupByName(gsl::czstring<> name) const noexcept { return NodeDevice{virNodeDeviceLookupByName(underlying, name)}; }

NodeDevice Connection::deviceLookupSCSIHostByWWN(gsl::czstring<> wwnn, gsl::czstring<> wwpn) const noexcept {
    return NodeDevice{virNodeDeviceLookupSCSIHostByWWN(underlying, wwnn, wwpn, 0)};
}

[[nodiscard]] inline auto Connection::listAllStoragePools(List::StoragePool::Flag flags) const noexcept {
    return meta::light::wrap_opram_owning_set_destroyable_arr<StoragePool>(underlying, virConnectListAllStoragePools, to_integral(flags));
}
[[nodiscard]] inline std::vector<StoragePool> Connection::extractAllStoragePools(List::StoragePool::Flag flags) const {
    return meta::heavy::wrap_opram_owning_set_destroyable_arr<StoragePool>(underlying, virConnectListAllStoragePools, to_integral(flags));
}
[[nodiscard]] inline auto Connection::listDefinedStoragePoolsNames() const noexcept {
    return meta::light::wrap_oparm_owning_fill_freeable_arr(underlying, virConnectNumOfDefinedStoragePools, virConnectListDefinedStoragePools);
}
[[nodiscard]] inline auto Connection::listStoragePoolsNames() const noexcept {
    return meta::light::wrap_oparm_owning_fill_freeable_arr(underlying, virConnectNumOfStoragePools, virConnectListStoragePools);
}
[[nodiscard]] inline int Connection::numOfDefinedStoragePools() const noexcept { return virConnectNumOfDefinedStoragePools(underlying); }
[[nodiscard]] inline int Connection::numOfStoragePools() const noexcept { return virConnectNumOfStoragePools(underlying); }

inline StoragePool Connection::storagePoolLookupByName(gsl::czstring<> name) const noexcept {
    return StoragePool{virStoragePoolLookupByName(underlying, name)};
}
inline StoragePool Connection::storagePoolLookupByTargetPath(gsl::czstring<> path) const noexcept {
    return StoragePool{virStoragePoolLookupByTargetPath(underlying, path)};
}
inline StoragePool Connection::storagePoolLookupByUUID(gsl::basic_zstring<const unsigned char> uuid) const noexcept {
    return StoragePool{virStoragePoolLookupByUUID(underlying, uuid)};
}
inline StoragePool Connection::storagePoolLookupByUUIDString(gsl::czstring<> uuidstr) const noexcept {
    return StoragePool{virStoragePoolLookupByUUIDString(underlying, uuidstr)};
}

[[nodiscard]] inline StorageVol Connection::storageVolLookupByKey(gsl::czstring<> key) const noexcept {
    return StorageVol{virStorageVolLookupByKey(underlying, key)};
}
[[nodiscard]] inline StorageVol Connection::storageVolLookupByPath(gsl::czstring<> path) const noexcept {
    return StorageVol{virStorageVolLookupByPath(underlying, path)};
}

inline bool Connection::restore(gsl::czstring<> from) noexcept { return virDomainRestore(underlying, from) == 0; }

inline bool Connection::restore(gsl::czstring<> from, gsl::czstring<> dxml, Domain::SaveRestoreFlag flags) noexcept {
    return virDomainRestoreFlags(underlying, from, dxml, to_integral(flags)) == 0;
}

constexpr inline Connection::Flags operator|(Connection::Flags lhs, Connection::Flags rhs) noexcept {
    return Connection::Flags(to_integral(lhs) | to_integral(rhs));
}

constexpr inline Connection::List::Domains::Flag operator|(Connection::List::Domains::Flag lhs, Connection::List::Domains::Flag rhs) noexcept {
    return Connection::List::Domains::Flag(EHTag{}, to_integral(lhs) | to_integral(rhs));
}
constexpr inline Connection::List::Domains::Flag& operator|=(Connection::List::Domains::Flag& lhs, Connection::List::Domains::Flag rhs) noexcept {
    return lhs = lhs | rhs;
}
[[nodiscard]] constexpr inline Connection::List::Networks::Flag operator|(Connection::List::Networks::Flag lhs,
                                                                          Connection::List::Networks::Flag rhs) noexcept {
    return Connection::List::Networks::Flag{to_integral(lhs) | to_integral(rhs)};
}
constexpr inline Connection::List::Networks::Flag& operator|=(Connection::List::Networks::Flag& lhs, Connection::List::Networks::Flag rhs) noexcept {
    return lhs = lhs | rhs;
}
constexpr inline Connection::GetAllDomains::Stats::Flags operator|(Connection::GetAllDomains::Stats::Flags lhs,
                                                                   Connection::GetAllDomains::Stats::Flags rhs) noexcept {
    return Connection::GetAllDomains::Stats::Flags(to_integral(lhs) | to_integral(rhs));
}

constexpr inline Connection::List::Devices::Flags operator|(Connection::List::Devices::Flags lhs, Connection::List::Devices::Flags rhs) noexcept {
    return Connection::List::Devices::Flags{to_integral(lhs) | to_integral(rhs)};
}
}