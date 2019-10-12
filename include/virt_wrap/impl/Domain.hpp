//
// Created by _as on 2019-02-01.
//

#pragma once

#include <algorithm>
#include <libvirt/libvirt.h>
#include "../Connection.hpp"
#include "../CpuMap.hpp"
//#include "../Domain.hpp"
#include "../Stream.hpp"
#include "../TypesParam.hpp"

namespace virt {
constexpr inline Domain::Domain(virDomainPtr ptr) noexcept : underlying(ptr) {}
constexpr inline Domain::Domain(Domain&& dom) noexcept : underlying(dom.underlying) { dom.underlying = nullptr; }

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
[[nodiscard]] inline Domain Domain::createXML(Connection& c, gsl::czstring<> xml) { return Domain{virDomainCreateXML(c.underlying, xml, 0)}; }

inline bool Domain::abortJob() noexcept { return virDomainAbortJob(underlying) == 0; }

inline bool Domain::addIOThread(unsigned int iothread_id, ModificationImpactFlag flags) noexcept {
    return virDomainAddIOThread(underlying, iothread_id, to_integral(flags)) == 0;
}

inline bool Domain::attachDevice(gsl::czstring<> xml) noexcept { return virDomainAttachDevice(underlying, xml) == 0; }

inline bool Domain::attachDevice(gsl::czstring<> xml, DeviceModifyFlag flags) noexcept {
    return virDomainAttachDeviceFlags(underlying, xml, to_integral(flags)) == 0;
}

inline bool Domain::blockCommit(gsl::czstring<> disk, gsl::czstring<> base, gsl::czstring<> top, unsigned long bandwidth,
                                BlockCommitFlag flags) noexcept {
    return virDomainBlockCommit(underlying, disk, base, top, bandwidth, to_integral(flags)) >= 0;
}

inline bool Domain::blockCopy(gsl::czstring<> disk, gsl::czstring<> destxml, const TypedParams& params, BlockCopyFlag flags) noexcept {
    return virDomainBlockCopy(underlying, disk, destxml, params.underlying, params.size, to_integral(flags)) >= 0;
}

inline bool Domain::blockJobAbort(gsl::czstring<> disk, BlockJobAbortFlag flags) noexcept {
    return virDomainBlockJobAbort(underlying, disk, to_integral(flags)) >= 0;
}

inline bool Domain::blockJobSetSpeed(gsl::czstring<> disk, unsigned long bandwidth, BlockJobSetSpeedFlag flags) noexcept {
    return virDomainBlockJobSetSpeed(underlying, disk, bandwidth, to_integral(flags)) >= 0;
}

inline bool Domain::blockPeek(gsl::czstring<> disk, unsigned long long offset, gsl::span<std::byte> buffer) const noexcept {
    return virDomainBlockPeek(underlying, disk, offset, buffer.size(), buffer.data(), 0) >= 0;
}

inline bool Domain::blockPull(gsl::czstring<> disk, unsigned long bandwidth, BlockPullFlag flags) noexcept {
    return virDomainBlockPull(underlying, disk, bandwidth, to_integral(flags)) >= 0;
}

inline bool Domain::blockRebase(gsl::czstring<> disk, gsl::czstring<> base, unsigned long bandwidth, BlockRebaseFlag flags) {
    return virDomainBlockRebase(underlying, disk, base, bandwidth, to_integral(flags)) >= 0;
}

inline bool Domain::blockResize(gsl::czstring<> disk, unsigned long long size, BlockResizeFlag flags) noexcept {
    return virDomainBlockResize(underlying, disk, size, to_integral(flags)) >= 0;
}

inline auto Domain::blockStats(gsl::czstring<> disk, size_t size) const noexcept {
    return meta::light::wrap_oparm_owning_fill_static_arr(
        underlying, [=](virDomainPtr) { return size; },
        [=](virDomainPtr u, virDomainBlockStatsPtr ptr, size_t n) { return virDomainBlockStats(u, disk, ptr, n); });
}

inline auto Domain::blockStatsFlags(gsl::czstring<> disk, TypedParameterFlag flags) const noexcept {
    return TPImpl::wrap_oparm_fill_tp(
        underlying, [=](virDomainPtr u, virTypedParameterPtr ptr, int* n) { return virDomainBlockStatsFlags(u, disk, ptr, n, to_integral(flags)); });
}

inline bool Domain::create() noexcept { return virDomainCreate(underlying) == 0; }

inline bool Domain::create(CreateFlag flags) noexcept { return virDomainCreateWithFlags(underlying, to_integral(flags)) == 0; }

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

[[nodiscard]] inline bool Domain::getAutostart() const noexcept {
    int val;
    virDomainGetAutostart(underlying, &val);
    return val == 1;
}

[[nodiscard]] inline auto Domain::getBlkioParameters(MITPFlags flags) const noexcept {
    return TPImpl::wrap_oparm_fill_tp(underlying, virDomainGetBlkioParameters, to_integral(flags));
}

[[nodiscard]] inline auto Domain::getBlockInfo(gsl::czstring<> disk) const noexcept -> std::optional<virDomainBlockInfo> {
    std::optional<virDomainBlockInfo> ret;
    auto& info = ret.emplace();
    return virDomainGetBlockInfo(underlying, disk, &info, 0) >= 0 ? ret : std::nullopt;
}

[[nodiscard]] inline auto Domain::getBlockIoTune(gsl::czstring<> disk, MITPFlags flags) const noexcept {
    return TPImpl::wrap_oparm_fill_tp(
        underlying, [=](auto* u, auto ptr, auto* n, auto f) { return virDomainGetBlockIoTune(u, disk, ptr, n, f); }, to_integral(flags));
}

[[nodiscard]] inline auto Domain::getBlockJobInfo(gsl::czstring<> disk, BlockJobInfoFlag flags) const noexcept {
    std::optional<virDomainBlockJobInfo> ret;
    auto& info = ret.emplace();
    return virDomainGetBlockJobInfo(underlying, disk, &info, to_integral(flags)) >= 0 ? ret : std::nullopt;
}

[[nodiscard]] Connection Domain::getConnect() const noexcept { return Connection{virDomainGetConnect(underlying)}; }

[[nodiscard]] std::optional<virDomainControlInfo> Domain::getControlInfo() const noexcept {
    virDomainControlInfo info;
    return virDomainGetControlInfo(underlying, &info, 0) == 0 ? std::optional{info} : std::nullopt;
}

[[nodiscard]] inline auto Domain::getTotalCPUStats() const noexcept {
    return TPImpl::wrap_oparm_fill_tp(underlying, [](auto u, auto tpu, int* size) {
        int res;
        return (res = virDomainGetCPUStats(u, tpu, *size, -1, 1, 0)), (static_cast<bool>(tpu) ? res : (*size = res));
    });
}
[[nodiscard]] inline auto Domain::getCPUStats(unsigned start_cpu, unsigned ncpus) const noexcept {
    return TPImpl::wrap_oparm_fill_tp(underlying, [=](auto u, auto tpu, int* size) {
        int res;
        return (res = virDomainGetCPUStats(u, tpu, *size, start_cpu, ncpus, 0)), (static_cast<bool>(tpu) ? res : (*size = res));
    });
}

[[nodiscard]] inline auto Domain::getDiskErrors() const noexcept {
    return meta::light::wrap_oparm_owning_fill_autodestroyable_arr<DiskError>(
        underlying, +[](decltype(underlying) u) { return virDomainGetDiskErrors(u, nullptr, 0, 0); },
        +[](decltype(underlying) u, virDomainDiskErrorPtr ptr, int n) { return virDomainGetDiskErrors(u, ptr, n, 0); });
}

[[nodiscard]] inline auto Domain::extractDiskErrors() const -> std::vector<DiskError> {
    return meta::heavy::wrap_oparm_owning_fill_autodestroyable_arr<DiskError>(
        underlying, +[](decltype(underlying) u) { return virDomainGetDiskErrors(u, nullptr, 0, 0); },
        +[](decltype(underlying) u, virDomainDiskErrorPtr ptr, int n) { return virDomainGetDiskErrors(u, ptr, n, 0); });
}

/*
[[nodiscard]] inline CpuMap Domain::getEmulatorPinInfo(std::size_t maplen, ModificationImpactFlag flags) const noexcept {
CpuMap ret;

}
*/
[[nodiscard]] inline auto Domain::getFSInfo() const noexcept {
    return meta::light::wrap_opram_owning_set_destroyable_arr<virDomainFSInfo, UniqueSpan, virDomainFSInfoFree>(underlying, virDomainGetFSInfo, 0u);
}

[[nodiscard]] inline auto Domain::extractFSInfo() const -> std::vector<FSInfo> {
    return meta::heavy::wrap_opram_owning_set_destroyable_arr<FSInfo>(underlying, virDomainGetFSInfo, 0u);
}

[[nodiscard]] inline auto Domain::getJobStats(GetJobStatsFlag flags) const noexcept {
    int jt = to_integral(JobType::NONE);
    auto res = TPImpl::wrap_oparm_set_tp(
        underlying, [&](auto* u, auto ptr, auto* n, auto f) { return virDomainGetJobStats(u, &jt, ptr, n, f); }, to_integral(flags));
    return std::make_pair(JobType{jt}, std::move(res));
}

[[nodiscard]] std::optional<TypedParams> Domain::getGuestVcpus() const noexcept {
    return TPImpl::wrap_oparm_set_tp(
        underlying,
        [](auto& u, auto ls, auto pcnt, auto... args) { return virDomainGetGuestVcpus(u, ls, reinterpret_cast<unsigned*>(pcnt), args...); }, 0);
}

[[nodiscard]] inline UniqueZstring Domain::getHostname() const noexcept { return UniqueZstring{virDomainGetHostname(underlying, 0)}; }

[[nodiscard]] inline std::string Domain::extractHostname() const noexcept { return {virDomainGetHostname(underlying, 0)}; }

[[nodiscard]] inline auto Domain::getIOThreadInfo(ModificationImpactFlag flags) const noexcept {
    return meta::light::wrap_opram_owning_set_destroyable_arr<light::IOThreadInfo>(underlying, virDomainGetIOThreadInfo,
                                                                                   static_cast<unsigned>(to_integral(flags)));
}

[[nodiscard]] inline auto Domain::extractIOThreadInfo(ModificationImpactFlag flags) const -> std::vector<heavy::IOThreadInfo> {
    return meta::heavy::wrap_opram_owning_set_destroyable_arr<heavy::IOThreadInfo>(underlying, virDomainGetIOThreadInfo,
                                                                                   static_cast<unsigned>(to_integral(flags)));
}

[[nodiscard]] inline Domain::Info Domain::getInfo() const noexcept {
    virDomainInfo info;
    virDomainGetInfo(underlying, &info);
    return info;
}

[[nodiscard]] inline auto Domain::getInterfaceParameters(gsl::czstring<> device, MITPFlags flags) const noexcept {
    return TPImpl::wrap_oparm_fill_tp(
        underlying, [=](auto u, auto&&... args) { return virDomainGetInterfaceParameters(u, device, args...); }, to_integral(flags));
}

[[nodiscard]] inline auto Domain::getJobInfo() const noexcept -> std::optional<JobInfo> {
    std::optional<JobInfo> ret{JobInfo{}};
    return virDomainGetJobInfo(underlying, &*ret) == 0 ? ret : std::nullopt;
}

[[nodiscard]] inline auto Domain::getLaunchSecurityInfo() const noexcept {
    return TPImpl::wrap_oparm_set_tp(underlying, virDomainGetLaunchSecurityInfo, 0);
}

[[nodiscard]] inline int Domain::getMaxVcpus() const noexcept { return virDomainGetMaxVcpus(underlying); }

[[nodiscard]] inline auto Domain::getMemoryParameters(MITPFlags flags) const noexcept {
    return TPImpl::wrap_oparm_fill_tp(underlying, virDomainGetMemoryParameters, to_integral(flags));
}

[[nodiscard]] inline UniqueZstring Domain::getMetadata(MetadataType type, gsl::czstring<> ns, ModificationImpactFlag flags) const noexcept {
    return UniqueZstring{virDomainGetMetadata(underlying, to_integral(type), ns, to_integral(flags))};
}

[[nodiscard]] inline std::string Domain::extractMetadata(MetadataType type, gsl::czstring<> ns, ModificationImpactFlag flags) const {
    return std::string{static_cast<const char*>(getMetadata(type, ns, flags))};
}

[[nodiscard]] inline gsl::czstring<> Domain::getName() const noexcept { return virDomainGetName(underlying); }

[[nodiscard]] inline auto Domain::getNumaParameters(MITPFlags flags) const noexcept {
    return TPImpl::wrap_oparm_fill_tp(underlying, virDomainGetNumaParameters, to_integral(flags));
}

[[nodiscard]] inline unsigned Domain::getID() const noexcept { return virDomainGetID(underlying); }

[[nodiscard]] inline int Domain::getNumVcpus(VCpuFlag flags) const noexcept { return virDomainGetVcpusFlags(underlying, to_integral(flags)); }

[[nodiscard]] inline auto Domain::getSchedulerType() const noexcept -> std::pair<UniqueZstring, int> {
    std::pair<UniqueZstring, int> ret{};
    ret.first = UniqueZstring{virDomainGetSchedulerType(underlying, &ret.second)};
    return ret;
}

[[nodiscard]] inline auto Domain::getSecurityLabel() const noexcept -> std::unique_ptr<virSecurityLabel> {
    auto ret = std::make_unique<virSecurityLabel>();
    return virDomainGetSecurityLabel(underlying, ret.get()) == 0 ? std::move(ret) : std::unique_ptr<virSecurityLabel>{nullptr};
}

[[nodiscard]] inline auto Domain::getSecurityLabelList() const noexcept {
    return meta::light::wrap_opram_owning_set_destroyable_arr<virSecurityLabel>(underlying, virDomainGetSecurityLabelList);
}

[[nodiscard]] inline auto Domain::extractSecurityLabelList() const -> std::vector<virSecurityLabel> {
    return meta::heavy::wrap_opram_owning_set_destroyable_arr(underlying, virDomainGetSecurityLabelList);
}

[[nodiscard]] inline Domain::StateWReason Domain::getState() const noexcept {
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

[[nodiscard]] inline auto Domain::getTime() const noexcept {
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

[[nodiscard]] inline std::string Domain::extractUUIDString() const {
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

[[nodiscard]] inline auto Domain::getPerfEvents(MITPFlags flags) const noexcept {
    return TPImpl::wrap_oparm_set_tp(underlying, virDomainGetPerfEvents, to_integral(flags));
}

[[nodiscard]] inline auto Domain::getSchedulerParameters() const noexcept {
    const auto sche_type = getSchedulerType();
    if (!sche_type.first)
        return std::optional<TypedParams>{std::nullopt};
    return TPImpl::wrap_oparm_fill_tp(
        underlying,
        [n = sche_type.second](auto*, std::nullptr_t, int* c) {
            *c = n;
            return 0;
        },
        virDomainGetSchedulerParameters);
}

[[nodiscard]] inline auto Domain::getSchedulerParameters(MITPFlags flags) const noexcept {
    const auto sche_type = getSchedulerType();
    if (!sche_type.first)
        return std::optional<TypedParams>{std::nullopt};
    return TPImpl::wrap_oparm_fill_tp(
        underlying,
        [n = sche_type.second](auto*, std::nullptr_t, int* c, auto) {
            *c = n;
            return 0;
        },
        virDomainGetSchedulerParametersFlags, to_integral(flags));
}

[[nodiscard]] inline auto Domain::getVcpuPinInfo(VCpuFlag flags) -> std::optional<std::vector<unsigned char>> {
    return meta::light::wrap_oparm_owning_fill_autodestroyable_arr(
        underlying, [&](decltype(underlying) u) -> int { return getInfo().nrVirtCpu; },
        [=](decltype(underlying) u, unsigned char* ptr, int n) { return virDomainGetVcpuPinInfo(u, n, ptr, VIR_CPU_MAPLEN(n), to_integral(flags)); });
}

[[nodiscard]] inline auto Domain::getVcpus() const noexcept {
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

[[nodiscard]] inline gsl::czstring<> Domain::getXMLDesc(XmlFlag flags) const noexcept { return virDomainGetXMLDesc(underlying, to_integral(flags)); }

[[nodiscard]] inline TFE Domain::hasManagedSaveImage() const noexcept { return TFE{virDomainHasManagedSaveImage(underlying, 0)}; }

[[nodiscard]] inline bool Domain::injectNMI() noexcept { return virDomainInjectNMI(underlying, 0) == 0; }

[[nodiscard]] inline auto Domain::interfaceAddressesView(InterfaceAddressesSource source) const noexcept {
    return meta::light::wrap_opram_owning_set_destroyable_arr<InterfaceView, UniqueSpan>(underlying, virDomainInterfaceAddresses, to_integral(source),
                                                                                         0u);
}

[[nodiscard]] inline auto Domain::interfaceAddresses(InterfaceAddressesSource source) const -> std::vector<Interface> {
    return meta::heavy::wrap_opram_owning_set_destroyable_arr<Interface>(underlying, virDomainInterfaceAddresses, to_integral(source), 0u);
}

[[nodiscard]] inline auto Domain::interfaceStats(gsl::czstring<> device) const noexcept -> std::optional<virDomainInterfaceStatsStruct> {
    std::optional<virDomainInterfaceStatsStruct> ret{virDomainInterfaceStatsStruct{}};
    auto& s = *ret;
    return virDomainInterfaceStats(underlying, device, &s, sizeof(std::remove_reference_t<decltype(s)>)) ? ret : std::nullopt;
}

[[nodiscard]] inline TFE Domain::isPersistent() const noexcept { return TFE{virDomainIsPersistent(underlying)}; }

[[nodiscard]] inline TFE Domain::isUpdated() const noexcept { return TFE{virDomainIsUpdated(underlying)}; }

inline bool Domain::PMSuspendForDuration(unsigned target, unsigned long long duration) noexcept {
    return virDomainPMSuspendForDuration(underlying, target, duration, 0) == 0;
}

inline bool Domain::PMWakeup() noexcept { return virDomainPMWakeup(underlying, 0) == 0; }

inline bool Domain::managedSave(SaveRestoreFlag flags) noexcept { return virDomainManagedSave(underlying, to_integral(flags)) == 0; }

inline bool Domain::managedSaveDefineXML(gsl::czstring<> dxml, SaveRestoreFlag flags) noexcept {
    return virDomainManagedSaveDefineXML(underlying, dxml, to_integral(flags)) == 0;
}

[[nodiscard]] inline UniqueZstring Domain::managedSaveGetXMLDesc(SaveImageXMLFlag flags) const noexcept {
    return UniqueZstring{virDomainManagedSaveGetXMLDesc(underlying, to_integral(flags))};
}

[[nodiscard]] inline std::string Domain::managedSaveExtractXMLDesc(SaveImageXMLFlag flags) const noexcept {
    return {static_cast<const char*>(managedSaveGetXMLDesc(flags))};
}

inline bool Domain::managedSaveRemove() noexcept { return virDomainManagedSaveRemove(underlying, 0) == 0; }

inline bool Domain::memoryPeek(unsigned long long start, gsl::span<unsigned char> buffer, MemoryFlag flags) const noexcept {
    return virDomainMemoryPeek(underlying, start, buffer.size(), buffer.data(), to_integral(flags)) == 0;
}

inline auto Domain::memoryStats(unsigned int nr_stats) const noexcept {
    return meta::light::wrap_oparm_owning_fill_autodestroyable_arr(
        underlying, [=](decltype(underlying)) { return nr_stats; }, virDomainMemoryStats, 0u);
}

[[nodiscard]] inline Domain Domain::migrate(Connection dconn, MigrateFlag flags, gsl::czstring<> dname, gsl::czstring<> uri,
                                            unsigned long bandwidth) noexcept {
    return Domain{virDomainMigrate(underlying, dconn.underlying, to_integral(flags), dname, uri, bandwidth)};
}

[[nodiscard]] inline Domain Domain::migrate(Connection dconn, gsl::czstring<> dxml, MigrateFlag flags, gsl::czstring<> dname, gsl::czstring<> uri,
                                            unsigned long bandwidth) noexcept {
    return Domain{virDomainMigrate2(underlying, dconn.underlying, dxml, to_integral(flags), dname, uri, bandwidth)};
}

[[nodiscard]] inline Domain Domain::migrate(Connection dconn, const TypedParams& params, MigrateFlag flags) noexcept {
    return Domain{virDomainMigrate3(underlying, dconn.underlying, params.underlying, params.size, to_integral(flags))};
}

inline bool Domain::migrateToURI(gsl::czstring<> duri, MigrateFlag flags, gsl::czstring<> dname, unsigned long bandwidth) noexcept {
    return virDomainMigrateToURI(underlying, duri, to_integral(flags), dname, bandwidth) >= 0;
}

inline bool Domain::migrateToURI(gsl::czstring<> dconnuri, gsl::czstring<> miguri, gsl::czstring<> dxml, MigrateFlag flags, gsl::czstring<> dname,
                                 unsigned long bandwidth) noexcept {
    return virDomainMigrateToURI2(underlying, dconnuri, miguri, dxml, to_integral(flags), dname, bandwidth) >= 0;
}

inline bool Domain::migrateToURI(gsl::czstring<> dconnuri, const TypedParams& params, MigrateFlag flags) noexcept {
    return virDomainMigrateToURI3(underlying, dconnuri, params.underlying, params.size, to_integral(flags)) >= 0;
}

[[nodiscard]] inline auto Domain::migrateGetCompressionCache() const noexcept -> std::optional<unsigned long long> {
    unsigned long long v;
    return virDomainMigrateGetCompressionCache(underlying, &v, 0) == 0 ? std::optional{v} : std::nullopt;
}

[[nodiscard]] inline auto Domain::migrateGetMaxDowntime() const noexcept -> std::optional<unsigned long long> {
    unsigned long long v;
    return virDomainMigrateGetMaxDowntime(underlying, &v, 0) == 0 ? std::optional{v} : std::nullopt;
}

[[nodiscard]] inline auto Domain::migrateGetMaxSpeed(unsigned int flags) const noexcept -> std::optional<unsigned long> {
    unsigned long v;
    return virDomainMigrateGetMaxSpeed(underlying, &v, to_integral(flags)) == 0 ? std::optional{v} : std::nullopt;
}

inline bool Domain::migrateSetCompressionCache(unsigned long long cacheSize) noexcept {
    return virDomainMigrateSetCompressionCache(underlying, cacheSize, 0) == 0;
}

inline bool Domain::migrateSetMaxDowntime(unsigned long long downtime) noexcept {
    return virDomainMigrateSetMaxDowntime(underlying, downtime, 0) == 0;
}

inline bool Domain::migrateSetMaxSpeed(unsigned long bandwidth, unsigned int flags) noexcept {
    return virDomainMigrateSetMaxSpeed(underlying, bandwidth, to_integral(flags)) == 0;
}

inline bool Domain::migrateStartPostCopy(unsigned int flags) noexcept { return virDomainMigrateStartPostCopy(underlying, to_integral(flags)) == 0; }

inline bool Domain::openChannel(gsl::czstring<> name, Stream& st, ChannelFlag flags) noexcept {
    return virDomainOpenChannel(underlying, name, st.underlying, to_integral(flags)) >= 0;
}

inline bool Domain::openConsole(gsl::czstring<> dev_name, Stream& st, ConsoleFlag flags) noexcept {
    return virDomainOpenConsole(underlying, dev_name, st.underlying, to_integral(flags)) >= 0;
}

inline bool Domain::openGraphics(unsigned int idx, int fd, OpenGraphicsFlag flags) const noexcept {
    return virDomainOpenGraphics(underlying, idx, fd, to_integral(flags)) >= 0;
}

[[nodiscard]] inline int Domain::openGraphicsFD(unsigned int idx, OpenGraphicsFlag flags) const noexcept {
    return virDomainOpenGraphicsFD(underlying, idx, to_integral(flags));
}

inline bool Domain::pinEmulator(CpuMap cpumap, ModificationImpactFlag flags) noexcept {
    return virDomainPinEmulator(underlying, cpumap.underlying, cpumap.maplen, to_integral(flags)) >= 0;
}

inline bool Domain::pinIOThread(unsigned int iothread_id, CpuMap cpumap, ModificationImpactFlag flags) noexcept {
    return virDomainPinIOThread(underlying, iothread_id, cpumap.underlying, cpumap.maplen, to_integral(flags)) >= 0;
}

inline bool Domain::pinVcpu(unsigned int vcpu, CpuMap cpumap) noexcept {
    return virDomainPinVcpu(underlying, vcpu, cpumap.underlying, cpumap.maplen) >= 0;
}

inline bool Domain::pinVcpuFlags(unsigned int vcpu, CpuMap cpumap, ModificationImpactFlag flags) noexcept {
    return virDomainPinVcpuFlags(underlying, vcpu, cpumap.underlying, cpumap.maplen, to_integral(flags)) >= 0;
}

inline bool Domain::sendKey(KeycodeSet codeset, unsigned int holdtime, gsl::span<const unsigned int> keycodes) noexcept {
    return virDomainSendKey(underlying, to_integral(codeset), holdtime, const_cast<unsigned int*>(keycodes.data()), keycodes.size(), 0) >= 0;
}

inline bool Domain::sendProcessSignal(long long pid_value, ProcessSignal signum) noexcept {
    return virDomainSendProcessSignal(underlying, pid_value, to_integral(signum), 0) >= 0;
}

inline bool Domain::setMaxMemory(unsigned long mem) { return virDomainSetMaxMemory(underlying, mem) == 0; }

inline bool Domain::setMemory(unsigned long mem) { return virDomainSetMemory(underlying, mem) == 0; }

inline bool Domain::setMemoryStatsPeriod(int period, MemoryModFlag flags) noexcept {
    return virDomainSetMemoryStatsPeriod(underlying, period, to_integral(flags)) >= 0;
}

[[nodiscard]] inline bool Domain::isActive() const noexcept { return virDomainIsActive(underlying) != 0; }

inline bool Domain::reboot(Domain::ShutdownFlag flags) { return virDomainReboot(underlying, to_integral(flags)) == 0; }
inline bool Domain::reboot() { return virDomainReboot(underlying, 0) == 0; }

inline bool Domain::rename(gsl::czstring<> name) { return virDomainRename(underlying, name, 0) == 0; }

inline bool Domain::reset() { return virDomainReset(underlying, 0) == 0; }

inline bool Domain::resume() noexcept { return virDomainResume(underlying) == 0; }

inline bool Domain::save(gsl::czstring<> to) noexcept { return virDomainSave(underlying, to) == 0; }

inline bool Domain::save(gsl::czstring<> to, gsl::czstring<> dxml, SaveRestoreFlag flags) noexcept {
    return virDomainSaveFlags(underlying, to, dxml, to_integral(flags)) == 0;
}

inline UniqueZstring Domain::screenshot(Stream& stream, unsigned int screen) const noexcept {
    return UniqueZstring{virDomainScreenshot(underlying, stream.underlying, screen, 0)};
}

inline bool Domain::setAutoStart(bool as) { return virDomainSetAutostart(underlying, as ? 1 : 0) == 0; }

inline bool Domain::setBlkioParameters(TypedParams params, ModificationImpactFlag flags) noexcept {
    return virDomainSetBlkioParameters(underlying, params.underlying, params.size, to_integral(flags)) >= 0;
}

inline bool Domain::setBlockIoTune(gsl::czstring<> disk, TypedParams params, ModificationImpactFlag flags) noexcept {
    return virDomainSetBlockIoTune(underlying, disk, params.underlying, params.size, to_integral(flags)) >= 0;
}

inline bool Domain::setBlockThreshold(gsl::czstring<> dev, unsigned long long threshold) noexcept {
    return virDomainSetBlockThreshold(underlying, dev, threshold, 0) >= 0;
}

inline bool Domain::setGuestVcpus(gsl::czstring<> cpumap, bool state) noexcept { return virDomainSetGuestVcpus(underlying, cpumap, state, 0) >= 0; }

inline bool Domain::setIOThreadParams(unsigned int iothread_id, TypedParams params, MITPFlags flags) noexcept {
    return virDomainSetIOThreadParams(underlying, iothread_id, params.underlying, params.size, to_integral(flags)) >= 0;
}

inline bool Domain::setInterfaceParameters(gsl::czstring<> device, TypedParams params, ModificationImpactFlag flags) noexcept {
    return virDomainSetInterfaceParameters(underlying, device, params.underlying, params.size, to_integral(flags)) >= 0;
}
inline bool Domain::setLifecycleAction(Lifecycle type, LifecycleAction action, ModificationImpactFlag flags) noexcept {
    return virDomainSetLifecycleAction(underlying, to_integral(type), to_integral(action), to_integral(flags)) >= 0;
}

inline bool Domain::setMemoryFlags(unsigned long memory, MemoryModFlag flags) noexcept {
    return virDomainSetMemoryFlags(underlying, memory, to_integral(flags)) >= 0;
}

inline bool Domain::setMemoryParameters(TypedParams params, ModificationImpactFlag flags) noexcept {
    return virDomainSetMemoryParameters(underlying, params.underlying, params.size, to_integral(flags)) >= 0;
}

inline bool Domain::setNumaParameters(TypedParams params, ModificationImpactFlag flags) noexcept {
    return virDomainSetNumaParameters(underlying, params.underlying, params.size, to_integral(flags)) >= 0;
}

inline bool Domain::setPerfEvents(TypedParams params, ModificationImpactFlag flags) noexcept {
    return virDomainSetPerfEvents(underlying, params.underlying, params.size, to_integral(flags)) >= 0;
}

inline bool Domain::setSchedulerParameters(TypedParams params) noexcept {
    return virDomainSetSchedulerParameters(underlying, params.underlying, params.size) >= 0;
}

inline bool Domain::setSchedulerParameters(TypedParams params, ModificationImpactFlag flags) noexcept {
    return virDomainSetSchedulerParametersFlags(underlying, params.underlying, params.size, to_integral(flags)) >= 0;
}

inline bool Domain::setMetadata(MetadataType type, gsl::czstring<> metadata, gsl::czstring<> key, gsl::czstring<> uri,
                                ModificationImpactFlag flags) noexcept {
    return virDomainSetMetadata(underlying, to_integral(type), metadata, key, uri, to_integral(flags)) >= 0;
}

inline bool Domain::setTime(long long seconds, unsigned int nseconds, Domain::SetTimeFlag flags) noexcept {
    return virDomainSetTime(underlying, seconds, nseconds, to_integral(flags));
}

inline bool Domain::setUserPassword(gsl::czstring<> user, gsl::czstring<> password, Domain::SetUserPasswordFlag flags) noexcept {
    return virDomainSetUserPassword(underlying, user, password, to_integral(flags));
}

inline bool Domain::setVcpu(gsl::czstring<> vcpumap, bool state, ModificationImpactFlag flags) noexcept {
    return virDomainSetVcpu(underlying, vcpumap, state, to_integral(flags)) >= 0;
}

inline bool Domain::setVcpus(unsigned int nvcpus) noexcept { return virDomainSetVcpus(underlying, nvcpus) == 0; }

inline bool Domain::setVcpus(unsigned int nvcpus, Domain::VCpuFlag flags) noexcept {
    return virDomainSetVcpusFlags(underlying, nvcpus, to_integral(flags)) == 0;
}

inline bool Domain::shutdown() noexcept { return virDomainShutdown(underlying) == 0; }

inline bool Domain::shutdown(Domain::ShutdownFlag flags) noexcept { return virDomainShutdownFlags(underlying, to_integral(flags)) == 0; }

inline bool Domain::suspend() noexcept { return virDomainSuspend(underlying) == 0; }

inline bool Domain::undefine() noexcept { return virDomainUndefine(underlying) == 0; }

inline bool Domain::undefine(UndefineFlag flags) noexcept { return virDomainUndefineFlags(underlying, to_integral(flags)) == 0; }

inline bool Domain::updateDeviceFlags(gsl::czstring<> xml, DeviceModifyFlag flags) noexcept {
    return virDomainUpdateDeviceFlags(underlying, xml, to_integral(flags)) >= 0;
}

inline Domain::Stats::Record::Record(const virDomainStatsRecord& from) noexcept : dom(from.dom) {
    params.reserve(static_cast<std::size_t>(from.nparams));
    std::transform(from.params, from.params + from.nparams, std::back_inserter(params),
                   [](const virTypedParameter& tp) { return TypedParameter{tp}; });
}
}