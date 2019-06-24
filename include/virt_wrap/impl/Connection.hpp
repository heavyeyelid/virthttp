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
#include "../fwd.hpp"
#include "../type_ops.hpp"
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

auto Connection::listAllDomains(List::Domains::Flags flags) const -> std::vector<Domain> {
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

Domain Connection::domainLookupByUUIDString(gsl::czstring<> uuid_str) const noexcept {
    return Domain{virDomainLookupByUUIDString(underlying, uuid_str)};
}

Network Connection::networkLookupByUUID(gsl::basic_zstring<const unsigned char> uuid) const noexcept { return Network{virNetworkLookupByUUID(underlying, uuid)}; }

Network Connection::networkLookupByName(gsl::czstring<> name) const noexcept { return Network{virNetworkLookupByName(underlying, name)}; }

Network Connection::networkLookupByUUIDString(gsl::czstring<> uuid_str) const noexcept {
    return Network{virNetworkLookupByUUIDString(underlying, uuid_str)};
}

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

auto Connection::listAllNetworks(std::optional<bool> active, std::optional<bool> persistent, std::optional<bool> autostart) const noexcept {
    auto flags = 0u;
    flags |= active ? (*active ? VIR_CONNECT_LIST_NETWORKS_ACTIVE : VIR_CONNECT_LIST_NETWORKS_INACTIVE) : 0u;
    flags |= persistent ? (*persistent ? VIR_CONNECT_LIST_NETWORKS_PERSISTENT : VIR_CONNECT_LIST_NETWORKS_TRANSIENT) : 0u;
    flags |= autostart ? (*autostart ? VIR_CONNECT_LIST_NETWORKS_AUTOSTART : VIR_CONNECT_LIST_NETWORKS_NO_AUTOSTART) : 0u;
    return virt::meta::light::wrap_opram_owning_set_destroyable_arr<Network>(underlying, virConnectListAllNetworks, flags);
}

inline auto Connection::listDefinedNetworksNames() const noexcept {
    return virt::meta::light::wrap_oparm_owning_fill_freeable_arr(underlying, virConnectNumOfDefinedNetworks, virConnectListDefinedNetworks);
}

auto Connection::listNetworksNames() const noexcept {
    return virt::meta::light::wrap_oparm_owning_fill_freeable_arr(underlying, virConnectNumOfNetworks, virConnectListNetworks);
}

auto Connection::extractDefinedNetworksNames() const -> std::vector<std::string> {
    return virt::meta::heavy::wrap_oparm_owning_fill_freeable_arr<std::string>(underlying, virConnectNumOfDefinedNetworks, virConnectListDefinedNetworks);
}

auto Connection::extractNetworksNames() const -> std::vector<std::string> {
    return virt::meta::heavy::wrap_oparm_owning_fill_freeable_arr<std::string>(underlying, virConnectNumOfNetworks, virConnectListNetworks);
}

std::vector<Network> Connection::extractAllNetworks(std::optional<bool> active, std::optional<bool> persistent, std::optional<bool> autostart) const {
    auto flags = 0u;
    flags |= active ? (*active ? VIR_CONNECT_LIST_NETWORKS_ACTIVE : VIR_CONNECT_LIST_NETWORKS_INACTIVE) : 0u;
    flags |= persistent ? (*persistent ? VIR_CONNECT_LIST_NETWORKS_PERSISTENT : VIR_CONNECT_LIST_NETWORKS_TRANSIENT) : 0u;
    flags |= autostart ? (*autostart ? VIR_CONNECT_LIST_NETWORKS_AUTOSTART : VIR_CONNECT_LIST_NETWORKS_NO_AUTOSTART) : 0u;
    return virt::meta::heavy::wrap_opram_owning_set_destroyable_arr<Network>(underlying, virConnectListAllNetworks, flags);
}

constexpr inline Connection::Flags operator|(Connection::Flags lhs, Connection::Flags rhs) noexcept {
    return Connection::Flags(to_integral(lhs) | to_integral(rhs));
}

constexpr inline Connection::List::Domains::Flags operator|(Connection::List::Domains::Flags lhs, Connection::List::Domains::Flags rhs) noexcept {
    return Connection::List::Domains::Flags(to_integral(lhs) | to_integral(rhs));
}

constexpr inline Connection::GetAllDomains::Stats::Flags operator|(Connection::GetAllDomains::Stats::Flags lhs,
                                                                   Connection::GetAllDomains::Stats::Flags rhs) noexcept {
    return Connection::GetAllDomains::Stats::Flags(to_integral(lhs) | to_integral(rhs));
}
}