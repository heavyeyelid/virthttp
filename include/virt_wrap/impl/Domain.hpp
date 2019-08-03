//
// Created by _as on 2019-02-01.
//

#pragma once

#include <algorithm>
#include <libvirt/libvirt.h>
#include "../Domain.hpp"
#include "../TypesParam.hpp"
#include "../type_ops.hpp"
#include "Connection.hpp"

namespace virt {
constexpr inline Domain::Domain(virDomainPtr ptr) noexcept : underlying(ptr) {}
constexpr inline Domain::Domain(Domain&& dom) noexcept : underlying(dom.underlying) {}

inline Domain& Domain::operator=(Domain&& d) noexcept {
    this->~Domain();
    underlying = d.underlying;
    d.underlying = nullptr;
    return *this;
}

inline Domain::~Domain() noexcept {
    if (*this)
        virDomainFree(underlying);
}

constexpr inline Domain::operator bool() const noexcept { return underlying != nullptr; }

[[nodiscard]] inline Domain Domain::createXML(Connection& c, gsl::czstring<> xml, CreateFlag flags) {
    return Domain{virDomainCreateXML(c.underlying, xml, to_integral(flags))};
}

bool Domain::abortJob() noexcept { return virDomainAbortJob(underlying) == 0; }

bool Domain::addIOThread(unsigned int iothread_id, ModificationImpactFlag flags) noexcept {
    return virDomainAddIOThread(underlying, iothread_id, to_integral(flags)) == 0;
}

bool Domain::attachDevice(gsl::czstring<> xml) noexcept { return virDomainAttachDevice(underlying, xml) == 0; }

bool Domain::attachDevice(gsl::czstring<> xml, DeviceModifyFlag flags) noexcept {
    return virDomainAttachDeviceFlags(underlying, xml, to_integral(flags)) == 0;
}

bool inline Domain::create() noexcept { return virDomainCreate(underlying) == 0; }

bool inline Domain::create(CreateFlag flags) noexcept { return virDomainCreateWithFlags(underlying, to_integral(flags)) == 0; }

inline bool Domain::coreDump(std::filesystem::path to, CoreDump::Flag flags) const noexcept {
    return virDomainCoreDump(underlying, to.c_str(), to_integral(flags)) == 0;
}

inline bool Domain::coreDump(std::filesystem::path to, CoreDump::Format format, CoreDump::Flag flags) const noexcept {
    return virDomainCoreDumpWithFormat(underlying, to.c_str(), to_integral(format), to_integral(flags)) == 0;
}

inline bool Domain::delIOThread(unsigned int iothread_id, ModificationImpactFlag flags) noexcept {
    return virDomainDelIOThread(underlying, iothread_id, to_integral(flags)) == 0;
}

inline bool Domain::destroy() noexcept { return virDomainDestroy(underlying) == 0; }

inline bool Domain::destroy(DestroyFlag flags) noexcept { return virDomainDestroyFlags(underlying, to_integral(flags)) == 0; }

inline bool Domain::detachDevice(gsl::czstring<> xml) noexcept { return virDomainDetachDevice(underlying, xml) == 0; }

inline bool Domain::detachDevice(gsl::czstring<> xml, DeviceModifyFlag flags) noexcept {
    return virDomainDetachDeviceFlags(underlying, xml, to_integral(flags)) == 0;
}

inline bool Domain::detachDeviceAlias(gsl::czstring<> alias, DeviceModifyFlag flags) noexcept {
    return virDomainDetachDeviceAlias(underlying, alias, to_integral(flags)) == 0;
}

inline int Domain::fsFreeze(gsl::span<gsl::czstring<>> mountpoints) noexcept {
    return virDomainFSFreeze(underlying, mountpoints.data(), mountpoints.size(), 0);
}

inline int Domain::fsThaw(gsl::span<gsl::czstring<>> mountpoints) noexcept {
    return virDomainFSThaw(underlying, mountpoints.data(), mountpoints.size(), 0);
}

inline bool Domain::fsTrim(gsl::czstring<> mountpoint, unsigned long long minimum) noexcept {
    return virDomainFSTrim(underlying, mountpoint, minimum, 0) == 0;
}

[[nodiscard]] bool Domain::getAutostart() const noexcept {
    int val;
    virDomainGetAutostart(underlying, &val);
    return val == 1;
}

[[nodiscard]] Connection Domain::getConnect() const noexcept { return Connection{virDomainGetConnect(underlying)}; }

[[nodiscard]] std::optional<virDomainControlInfo> Domain::getControlInfo() const noexcept {
    virDomainControlInfo info;
    return virDomainGetControlInfo(underlying, &info, 0) == 0 ? std::optional{info} : std::nullopt;
}

[[nodiscard]] auto Domain::getDiskErrors() const noexcept {
    return meta::light::wrap_oparm_owning_fill_autodestroyable_arr<DiskError>(
        underlying, +[](decltype(underlying) u) { return virDomainGetDiskErrors(u, nullptr, 0, 0); },
        +[](decltype(underlying) u, virDomainDiskErrorPtr ptr, int n) { return virDomainGetDiskErrors(u, ptr, n, 0); });
}

[[nodiscard]] auto Domain::extractDiskErrors() const -> std::vector<DiskError> {
    return meta::heavy::wrap_oparm_owning_fill_autodestroyable_arr<DiskError>(
        underlying, +[](decltype(underlying) u) { return virDomainGetDiskErrors(u, nullptr, 0, 0); },
        +[](decltype(underlying) u, virDomainDiskErrorPtr ptr, int n) { return virDomainGetDiskErrors(u, ptr, n, 0); });
}

[[nodiscard]] auto Domain::getFSInfo() const noexcept {
    return meta::light::wrap_opram_owning_set_destroyable_arr<virDomainFSInfo, virDomainFSInfoFree>(underlying, virDomainGetFSInfo, 0u);
}

[[nodiscard]] auto Domain::extractFSInfo() const -> std::vector<FSInfo> {
    return meta::heavy::wrap_opram_owning_set_destroyable_arr<FSInfo>(underlying, virDomainGetFSInfo, 0u);
}

[[nodiscard]] inline UniqueZstring Domain::getHostname() const noexcept { return UniqueZstring{virDomainGetHostname(underlying, 0)}; }

[[nodiscard]] inline std::string Domain::extractHostname() const noexcept { return {virDomainGetHostname(underlying, 0)}; }

[[nodiscard]] auto Domain::getIOThreadInfo(ModificationImpactFlag flags) const noexcept {
    return meta::light::wrap_opram_owning_set_destroyable_arr<light::IOThreadInfo>(underlying, virDomainGetIOThreadInfo,
                                                                                   static_cast<unsigned>(to_integral(flags)));
}

[[nodiscard]] auto Domain::extractIOThreadInfo(ModificationImpactFlag flags) const -> std::vector<heavy::IOThreadInfo> {
    return meta::heavy::wrap_opram_owning_set_destroyable_arr<heavy::IOThreadInfo>(underlying, virDomainGetIOThreadInfo,
                                                                                   static_cast<unsigned>(to_integral(flags)));
}

[[nodiscard]] inline Domain::Info Domain::getInfo() const noexcept {
    virDomainInfo info;
    virDomainGetInfo(underlying, &info);
    return info;
}

[[nodiscard]] auto Domain::getJobInfo() const noexcept -> std::optional<JobInfo> {
    std::optional<JobInfo> ret{JobInfo{}};
    return virDomainGetJobInfo(underlying, &*ret) == 0 ? ret : std::nullopt;
}

[[nodiscard]] int Domain::getMaxVcpus() const noexcept { return virDomainGetMaxVcpus(underlying); }

[[nodiscard]] UniqueZstring Domain::getMetadata(MetadataType type, gsl::czstring<> ns, ModificationImpactFlag flags) const noexcept {
    return UniqueZstring{virDomainGetMetadata(underlying, to_integral(type), ns, to_integral(flags))};
}

[[nodiscard]] std::string Domain::extractMetadata(MetadataType type, gsl::czstring<> ns, ModificationImpactFlag flags) const {
    return std::string{static_cast<const char*>(getMetadata(type, ns, flags))};
}

[[nodiscard]] inline gsl::czstring<> Domain::getName() const noexcept { return virDomainGetName(underlying); }

[[nodiscard]] inline unsigned Domain::getID() const noexcept { return virDomainGetID(underlying); }

[[nodiscard]] int Domain::getNumVcpus(VCpuFlag flags) const noexcept { return virDomainGetVcpusFlags(underlying, to_integral(flags)); }

[[nodiscard]] auto Domain::getSchedulerType() const noexcept -> std::pair<UniqueZstring, int> {
    std::pair<UniqueZstring, int> ret{};
    ret.first = UniqueZstring{virDomainGetSchedulerType(underlying, &ret.second)};
    return ret;
}

[[nodiscard]] auto Domain::getSecurityLabel() const noexcept -> std::unique_ptr<virSecurityLabel> {
    auto ret = std::make_unique<virSecurityLabel>();
    return virDomainGetSecurityLabel(underlying, ret.get()) == 0 ? std::move(ret) : std::unique_ptr<virSecurityLabel>{nullptr};
}

[[nodiscard]] auto Domain::getSecurityLabelList() const noexcept {
    return meta::light::wrap_opram_owning_set_destroyable_arr<virSecurityLabel>(underlying, virDomainGetSecurityLabelList);
}

[[nodiscard]] auto Domain::extractSecurityLabelList() const -> std::vector<virSecurityLabel> {
    return meta::heavy::wrap_opram_owning_set_destroyable_arr(underlying, virDomainGetSecurityLabelList);
}

[[nodiscard]] Domain::StateWReason Domain::getState() const noexcept {
    int state;
    int reason;
    const auto res = virDomainGetState(underlying, &state, &reason, 0);
    if (res != 0)
        return StateWReason{};
    switch (state) {
    case VIR_DOMAIN_NOSTATE:
        return StateWReason{StateReason::NoState{reason}};
    case VIR_DOMAIN_RUNNING:
        return StateWReason{StateReason::Running{reason}};
    case VIR_DOMAIN_BLOCKED:
        return StateWReason{StateReason::Blocked{reason}};
    case VIR_DOMAIN_PAUSED:
        return StateWReason{StateReason::Paused{reason}};
    case VIR_DOMAIN_SHUTDOWN:
        return StateWReason{StateReason::Shutdown{reason}};
    case VIR_DOMAIN_SHUTOFF:
        return StateWReason{StateReason::Shutoff{reason}};
    case VIR_DOMAIN_CRASHED:
        return StateWReason{StateReason::Crashed{reason}};
    case VIR_DOMAIN_PMSUSPENDED:
        return StateWReason{StateReason::PMSuspended{reason}};
    }
    UNREACHABLE;
}

[[nodiscard]] auto Domain::getTime() const noexcept {
    struct TimeRet {
        long long seconds;
        unsigned nanosec;
    };
    std::optional<TimeRet> ret{};
    auto& time = ret.emplace();
    return virDomainGetTime(underlying, &time.seconds, &time.nanosec, 0) == 0 ? ret : std::nullopt;
}

[[nodiscard]] inline auto Domain::getUUID() const {
    using RetType = std::array<unsigned char, VIR_UUID_BUFLEN>;
    RetType ret{};
    const auto res = virDomainGetUUID(underlying, ret.data());
    if (res < 0)
        throw std::runtime_error{"virDomainGetUUID"};
    return ret;
}

[[nodiscard]] inline auto Domain::getUUIDString() const noexcept -> std::optional<std::array<char, VIR_UUID_STRING_BUFLEN>> {
    std::array<char, VIR_UUID_STRING_BUFLEN> ret{};
    return virDomainGetUUIDString(underlying, ret.data()) == 0 ? std::optional(ret) : std::nullopt;
}

[[nodiscard]] std::string Domain::extractUUIDString() const {
    std::string ret{};
    ret.resize(VIR_UUID_STRING_BUFLEN);
    virDomainGetUUIDString(underlying, ret.data());
    ret.resize(std::strlen(ret.data()));
    ret.shrink_to_fit();
    return ret;
}

[[nodiscard]] inline auto Domain::getOSType() const {
    return std::unique_ptr<char[], void (*)(char*)>{virDomainGetOSType(underlying), freeany<char[]>};
}

[[nodiscard]] inline unsigned long Domain::getMaxMemory() const noexcept { return virDomainGetMaxMemory(underlying); }

[[nodiscard]] auto Domain::getVcpuPinInfo(VCpuFlag flags) -> std::optional<std::vector<unsigned char>> {
    return meta::light::wrap_oparm_owning_fill_autodestroyable_arr(
        underlying, [&](decltype(underlying) u) -> int { return getInfo().nrVirtCpu; },
        [=](decltype(underlying) u, unsigned char* ptr, int n) { return virDomainGetVcpuPinInfo(u, n, ptr, VIR_CPU_MAPLEN(n), to_integral(flags)); });
}

[[nodiscard]] auto Domain::getVcpus() const noexcept {
    struct Ret {
        std::vector<virVcpuInfo> infos{};
        std::vector<unsigned char> cpumaps{};
        int cpumap_size{};
    };
    std::optional<Ret> ret{Ret{}};
    auto& st = *ret;
    const auto nvcpu = getInfo().nrVirtCpu;
    st.cpumap_size = VIR_CPU_MAPLEN(nvcpu);
    st.infos.resize(nvcpu);
    st.cpumaps.resize(st.cpumap_size * nvcpu);
    const auto res = virDomainGetVcpus(underlying, st.infos.data(), nvcpu, st.cpumaps.data(), VIR_CPU_MAPLEN(nvcpu));
    return res >= 0 ? ret : std::nullopt;
}

[[nodiscard]] gsl::czstring<> Domain::getXMLDesc(XmlFlag flags) const noexcept { return virDomainGetXMLDesc(underlying, to_integral(flags)); }

[[nodiscard]] TFE Domain::hasManagedSaveImage() const noexcept { return TFE{virDomainHasManagedSaveImage(underlying, 0)}; }

[[nodiscard]] bool Domain::injectNMI() noexcept { return virDomainInjectNMI(underlying, 0) == 0; }

[[nodiscard]] auto Domain::interfaceAddressesView(InterfaceAddressesSource source) const noexcept {
    return meta::light::wrap_opram_owning_set_autodestroyable_arr<InterfaceView>(underlying, virDomainInterfaceAddresses, to_integral(source), 0u);
}

[[nodiscard]] auto Domain::interfaceAddresses(InterfaceAddressesSource source) const -> std::vector<Interface> {
    return meta::heavy::wrap_opram_owning_set_destroyable_arr<Interface>(underlying, virDomainInterfaceAddresses, to_integral(source), 0u);
}

[[nodiscard]] auto Domain::interfaceStats(gsl::czstring<> device) const noexcept -> std::optional<virDomainInterfaceStatsStruct> {
    std::optional<virDomainInterfaceStatsStruct> ret{virDomainInterfaceStatsStruct{}};
    auto& s = *ret;
    return virDomainInterfaceStats(underlying, device, &s, sizeof(std::remove_reference_t<decltype(s)>)) ? ret : std::nullopt;
}

[[nodiscard]] TFE Domain::isPersistent() const noexcept { return TFE{virDomainIsPersistent(underlying)}; }

[[nodiscard]] TFE Domain::isUpdated() const noexcept { return TFE{virDomainIsUpdated(underlying)}; }

bool Domain::PMSuspendForDuration(unsigned target, unsigned long long duration) noexcept {
    return virDomainPMSuspendForDuration(underlying, target, duration, 0) == 0;
}

bool Domain::PMWakeup() noexcept { return virDomainPMWakeup(underlying, 0) == 0; }

bool Domain::managedSave(SaveRestoreFlag flags) noexcept { return virDomainManagedSave(underlying, to_integral(flags)) == 0; }

bool Domain::managedSaveDefineXML(gsl::czstring<> dxml, SaveRestoreFlag flags) noexcept {
    return virDomainManagedSaveDefineXML(underlying, dxml, to_integral(flags)) == 0;
}

[[nodiscard]] UniqueZstring Domain::managedSaveGetXMLDesc(SaveImageXMLFlag flags) const noexcept {
    return UniqueZstring{virDomainManagedSaveGetXMLDesc(underlying, to_integral(flags))};
}

[[nodiscard]] std::string Domain::managedSaveExtractXMLDesc(SaveImageXMLFlag flags) const noexcept {
    return {static_cast<const char*>(managedSaveGetXMLDesc(flags))};
}

bool Domain::managedSaveRemove() noexcept { return virDomainManagedSaveRemove(underlying, 0) == 0; }

bool Domain::memoryPeek(unsigned long long start, gsl::span<unsigned char> buffer, MemoryFlag flags) const noexcept {
    return virDomainMemoryPeek(underlying, start, buffer.size(), buffer.data(), to_integral(flags)) == 0;
}

auto Domain::memoryStats(unsigned int nr_stats) const noexcept {
    return meta::light::wrap_oparm_owning_fill_autodestroyable_arr(
        underlying, [=](decltype(underlying)) { return nr_stats; }, virDomainMemoryStats, 0u);
}

[[nodiscard]] auto Domain::migrateGetCompressionCache() const noexcept -> std::optional<unsigned long long> {
    unsigned long long v;
    return virDomainMigrateGetCompressionCache(underlying, &v, 0) == 0 ? std::optional{v} : std::nullopt;
}

[[nodiscard]] auto Domain::migrateGetMaxDowntime() const noexcept -> std::optional<unsigned long long> {
    unsigned long long v;
    return virDomainMigrateGetMaxDowntime(underlying, &v, 0) == 0 ? std::optional{v} : std::nullopt;
}

[[nodiscard]] auto Domain::migrateGetMaxSpeed(unsigned int flags) const noexcept -> std::optional<unsigned long> {
    unsigned long v;
    return virDomainMigrateGetMaxSpeed(underlying, &v, to_integral(flags)) == 0 ? std::optional{v} : std::nullopt;
}

bool Domain::migrateSetCompressionCache(unsigned long long cacheSize) noexcept {
    return virDomainMigrateSetCompressionCache(underlying, cacheSize, 0) == 0;
}

bool Domain::migrateSetMaxDowntime(unsigned long long downtime) noexcept { return virDomainMigrateSetMaxDowntime(underlying, downtime, 0) == 0; }

bool Domain::migrateSetMaxSpeed(unsigned long bandwidth, unsigned int flags) noexcept {
    return virDomainMigrateSetMaxSpeed(underlying, bandwidth, to_integral(flags)) == 0;
}

bool Domain::migrateStartPostCopy(unsigned int flags) noexcept { return virDomainMigrateStartPostCopy(underlying, to_integral(flags)) == 0; }

inline bool Domain::setMaxMemory(unsigned long mem) { return virDomainSetMaxMemory(underlying, mem) == 0; }

inline bool Domain::setMemory(unsigned long mem) { return virDomainSetMemory(underlying, mem) == 0; }

[[nodiscard]] inline bool Domain::isActive() const noexcept { return virDomainIsActive(underlying) != 0; }

inline bool Domain::reboot(Domain::ShutdownFlag flags) { return virDomainReboot(underlying, to_integral(flags)) == 0; }

inline bool Domain::rename(gsl::czstring<> name) { return virDomainRename(underlying, name, 0) == 0; }

inline bool Domain::reset() { return virDomainReset(underlying, 0) == 0; }

inline bool Domain::resume() noexcept { return virDomainResume(underlying) == 0; }

inline bool Domain::setAutoStart(bool as) { return virDomainSetAutostart(underlying, as ? 1 : 0) == 0; }

inline bool Domain::shutdown() noexcept { return virDomainShutdown(underlying) == 0; }

inline bool Domain::shutdown(Domain::ShutdownFlag flags) noexcept { return virDomainShutdownFlags(underlying, to_integral(flags)) == 0; }

inline void Domain::suspend() { virDomainSuspend(underlying); }

inline bool Domain::undefine() noexcept { return virDomainUndefine(underlying) == 0; }

inline bool Domain::undefine(UndefineFlags flags) noexcept { return virDomainUndefineFlags(underlying, to_integral(flags)) == 0; }

inline Domain::Stats::Record::Record(const virDomainStatsRecord& from) noexcept : dom(from.dom) {
    params.reserve(static_cast<std::size_t>(from.nparams));
    std::transform(from.params, from.params + from.nparams, std::back_inserter(params),
                   [](const virTypedParameter& tp) { return TypedParameter{tp}; });
}

[[nodiscard]] constexpr inline Domain::CoreDump::Flag operator|(Domain::CoreDump::Flag lhs, Domain::CoreDump::Flag rhs) noexcept {
    return Domain::CoreDump::Flag{to_integral(lhs) | to_integral(rhs)};
}

[[nodiscard]] constexpr inline Domain::GetAllDomainStatsFlag operator|(Domain::GetAllDomainStatsFlag lhs,
                                                                        Domain::GetAllDomainStatsFlag rhs) noexcept {
    return Domain::GetAllDomainStatsFlag{to_integral(lhs) | to_integral(rhs)};
}

constexpr inline Domain::GetAllDomainStatsFlag operator|=(Domain::GetAllDomainStatsFlag& lhs,
                                                                         Domain::GetAllDomainStatsFlag rhs) noexcept {
    return lhs = Domain::GetAllDomainStatsFlag{to_integral(lhs) | to_integral(rhs)};
}
[[nodiscard]] constexpr inline Domain::ShutdownFlag operator|(Domain::ShutdownFlag lhs, Domain::ShutdownFlag rhs) noexcept {
    return Domain::ShutdownFlag{to_integral(lhs) | to_integral(rhs)};
}
constexpr inline Domain::ShutdownFlag operator|=(Domain::ShutdownFlag& lhs, Domain::ShutdownFlag rhs) noexcept {
    return Domain::ShutdownFlag{to_integral(lhs) | to_integral(rhs)};
}
[[nodiscard]] constexpr inline Domain::StatsType operator|(Domain::StatsType lhs, Domain::StatsType rhs) noexcept {
    return Domain::StatsType{to_integral(lhs) | to_integral(rhs)};
}

constexpr inline Domain::StatsType operator|=(Domain::StatsType& lhs, Domain::StatsType rhs) noexcept {
    return lhs = Domain::StatsType{to_integral(lhs) | to_integral(rhs)};
}

[[nodiscard]] constexpr inline Domain::ModificationImpactFlag operator|(Domain::ModificationImpactFlag lhs,
                                                                         Domain::ModificationImpactFlag rhs) noexcept {
    return Domain::ModificationImpactFlag{to_integral(lhs) | to_integral(rhs)};
}
[[nodiscard]] constexpr inline Domain::VCpuFlag operator|(Domain::VCpuFlag lhs, Domain::VCpuFlag rhs) noexcept {
    return Domain::VCpuFlag{to_integral(lhs) | to_integral(rhs)};
}
constexpr inline Domain::VCpuFlag operator|=(Domain::VCpuFlag& lhs, Domain::VCpuFlag rhs) noexcept {
    return lhs = Domain::VCpuFlag{to_integral(lhs) | to_integral(rhs)};
}
[[nodiscard]] constexpr inline Domain::Stats::Types operator|(Domain::Stats::Types lhs, Domain::Stats::Types rhs) noexcept {
    return Domain::Stats::Types(to_integral(lhs) | to_integral(rhs));
}
}