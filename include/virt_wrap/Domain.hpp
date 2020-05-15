//
// Created by _as on 2019-01-31.
//

#ifndef VIRTPP_DOMAIN_HPP
#define VIRTPP_DOMAIN_HPP

#include <filesystem>
#include <stdexcept>
#include <variant>
#include <vector>
#include <gsl/gsl>
#include "../cexpr_algs.hpp"
#include "enums/Connection/Decls.hpp"
#include "enums/Domain/Decls.hpp"
#include "enums/Domain/Domain.hpp"
#include "enums/GFlags.hpp"
#include "CpuMap.hpp"
#include "fwd.hpp"
#include "tfe.hpp"
#include "utility.hpp"

namespace tmp {
/*
using virConnectDomainEventAgentLifecycleCallback = void (*)(virConnectPtr conn, virDomainPtr dom, int state, int reason, void* opaque);

using virConnectDomainEventBalloonChangeCallback = void (*)(virConnectPtr conn, virDomainPtr dom, unsigned long long actual, void* opaque);

using virConnectDomainEventBlockJobCallback = void (*)(virConnectPtr conn, virDomainPtr dom, const char* disk, int type, int status, void* opaque);

using virConnectDomainEventBlockThresholdCallback = void (*)(virConnectPtr conn, virDomainPtr dom, const char* dev, const char* path,
                                                         unsigned long long threshold, unsigned long long excess, void* opaque);

using virConnectDomainEventCallback = int (*)(virConnectPtr conn, virDomainPtr dom, int event, int detail, void* opaque);

int virConnectDomainEventDeregister(virConnectPtr conn, virConnectDomainEventCallback cb);
int virConnectDomainEventDeregisterAny(virConnectPtr conn, int callbackID);
using virConnectDomainEventDeviceAddedCallback = void (*)(virConnectPtr conn, virDomainPtr dom, const char* devAlias, void* opaque);

using virConnectDomainEventDeviceRemovalFailedCallback = void (*)(virConnectPtr conn, virDomainPtr dom, const char* devAlias, void* opaque);

using virConnectDomainEventDeviceRemovedCallback = void (*)(virConnectPtr conn, virDomainPtr dom, const char* devAlias, void* opaque);

using virConnectDomainEventDiskChangeCallback = void (*)(virConnectPtr conn, virDomainPtr dom, const char* oldSrcPath, const char* newSrcPath,
                                                     const char* devAlias, int reason, void* opaque);

using virConnectDomainEventGenericCallback = void (*)(virConnectPtr conn, virDomainPtr dom, void* opaque);

using virConnectDomainEventGraphicsCallback = void (*)(virConnectPtr conn, virDomainPtr dom, int phase, const virDomainEventGraphicsAddress* local,
                                                   const virDomainEventGraphicsAddress* remote, const char* authScheme,
                                                   const virDomainEventGraphicsSubject* subject, void* opaque);

using virConnectDomainEventIOErrorCallback = void (*)(virConnectPtr conn, virDomainPtr dom, const char* srcPath, const char* devAlias, int action,
                                                  void* opaque);

using virConnectDomainEventIOErrorReasonCallback = void (*)(virConnectPtr conn, virDomainPtr dom, const char* srcPath, const char* devAlias,
                                                        int action, const char* reason, void* opaque);

using virConnectDomainEventJobCompletedCallback = void (*)(virConnectPtr conn, virDomainPtr dom, virTypedParameterPtr params, int nparams,
                                                       void* opaque);

using virConnectDomainEventMetadataChangeCallback = void (*)(virConnectPtr conn, virDomainPtr dom, int type, const char* nsuri, void* opaque);

using virConnectDomainEventMigrationIterationCallback = void (*)(virConnectPtr conn, virDomainPtr dom, int iteration, void* opaque);

using virConnectDomainEventPMSuspendCallback = void (*)(virConnectPtr conn, virDomainPtr dom, int reason, void* opaque);

using virConnectDomainEventPMSuspendDiskCallback = void (*)(virConnectPtr conn, virDomainPtr dom, int reason, void* opaque);

using virConnectDomainEventPMWakeupCallback = void (*)(virConnectPtr conn, virDomainPtr dom, int reason, void* opaque);

using virConnectDomainEventRTCChangeCallback = void (*)(virConnectPtr conn, virDomainPtr dom, long long utcoffset, void* opaque);

int virConnectDomainEventRegister(virConnectPtr conn, virConnectDomainEventCallback cb, void* opaque, virFreeCallback freecb);
int virConnectDomainEventRegisterAny(virConnectPtr conn, virDomainPtr dom, int eventID, virConnectDomainEventGenericCallback cb, void* opaque,
                                 virFreeCallback freecb);
using virConnectDomainEventTrayChangeCallback = void (*)(virConnectPtr conn, virDomainPtr dom, const char* devAlias, int reason, void* opaque);

using virConnectDomainEventTunableCallback = void (*)(virConnectPtr conn, virDomainPtr dom, virTypedParameterPtr params, int nparams, void* opaque);

using virConnectDomainEventWatchdogCallback = void (*)(virConnectPtr conn, virDomainPtr dom, int action, void* opaque);

char* virConnectDomainXMLFromNative(virConnectPtr conn, const char* nativeFormat, const char* nativeConfig, unsigned int flags);
char* virConnectDomainXMLToNative(virConnectPtr conn, const char* nativeFormat, const char* domainXml, unsigned int flags);
char* virConnectGetDomainCapabilities(virConnectPtr conn, const char* emulatorbin, const char* arch, const char* machine, const char* virttype,
                                  unsigned int flags);
                                  */

/*
 * ((?:[A-Z]+_)+([A-Z]+)(?!_))\s*=.*,(.*)
 * $2 = $1, $3
 * */
} // namespace tmp

namespace virt {
class Domain {
    friend Connection;

    virDomainPtr underlying = nullptr;

  public:
    using Info = virDomainInfo;


    class StatsRecord;

    struct StateReason {};
    struct StateWReason;

    struct DiskError;
    struct FSInfo;
    struct Interface;
    struct InterfaceView;
    struct IPAddress;
    struct IPAddressView;
    struct JobInfo;
    struct light {
        struct IOThreadInfo;
    };
    struct heavy {
        struct IOThreadInfo;
    };

    using BlockStats = virDomainBlockStatsStruct;

    constexpr inline explicit Domain(virDomainPtr ptr = nullptr) noexcept;

    Domain(const Domain&) = delete;

    constexpr inline Domain(Domain&&) noexcept;

    Domain& operator=(const Domain&) = delete;

    inline Domain& operator=(Domain&&) noexcept;

    inline ~Domain() noexcept;

    constexpr inline explicit operator bool() const noexcept;

    bool abortJob() noexcept;

    bool addIOThread(unsigned int iothread_id, enums::domain::ModificationImpactFlag flags) noexcept;

    bool attachDevice(gsl::czstring<> xml) noexcept;

    bool attachDevice(gsl::czstring<> xml, enums::domain::DeviceModifyFlag flags) noexcept;

    bool blockCommit(gsl::czstring<> disk, gsl::czstring<> base, gsl::czstring<> top, unsigned long bandwidth,
                     enums::domain::BlockCommitFlag flags) noexcept;

    bool blockCopy(gsl::czstring<> disk, gsl::czstring<> destxml, const TypedParams& params, enums::domain::BlockCopyFlag flags) noexcept;

    bool blockJobAbort(gsl::czstring<> disk, enums::domain::BlockJobAbortFlag flags) noexcept;

    bool blockJobSetSpeed(gsl::czstring<> disk, unsigned long bandwidth, enums::domain::BlockJobSetSpeedFlag flags) noexcept;

    bool blockPeek(gsl::czstring<> disk, unsigned long long offset, gsl::span<std::byte> buffer) const noexcept;

    bool blockPull(gsl::czstring<> disk, unsigned long bandwidth, enums::domain::BlockPullFlag flags) noexcept;

    bool blockRebase(gsl::czstring<> disk, gsl::czstring<> base, unsigned long bandwidth, enums::domain::BlockRebaseFlag flags);

    bool blockResize(gsl::czstring<> disk, unsigned long long size, enums::domain::BlockResizeFlag flags) noexcept;

    auto blockStats(gsl::czstring<> disk, size_t size) const noexcept;

    auto blockStatsFlags(gsl::czstring<> disk, enums::TypedParameterFlag flags) const noexcept;

    bool create() noexcept;

    bool create(enums::domain::CreateFlag flag) noexcept;

    // createWithFiles() // Left out

    bool coreDump(std::filesystem::path to, enums::domain::core_dump::Flag flags) const noexcept;

    bool coreDump(std::filesystem::path to, enums::domain::core_dump::Format format, enums::domain::core_dump::Flag flags) const noexcept;

    bool delIOThread(unsigned int iothread_id, enums::domain::ModificationImpactFlag flags) noexcept;

    bool destroy() noexcept;

    bool destroy(enums::domain::DestroyFlag flag) noexcept;

    bool detachDevice(gsl::czstring<> xml) noexcept;

    bool detachDevice(gsl::czstring<> xml, enums::domain::DeviceModifyFlag flag) noexcept;

    bool detachDeviceAlias(gsl::czstring<> alias, enums::domain::DeviceModifyFlag flag) noexcept;

    int fsFreeze(gsl::span<gsl::czstring<>> mountpoints) noexcept;

    int fsThaw(gsl::span<gsl::czstring<>> mountpoints) noexcept;

    bool fsTrim(gsl::czstring<> mountpoint, unsigned long long minimum) noexcept;

    [[nodiscard]] bool getAutostart() const noexcept;

    [[nodiscard]] auto getBlkioParameters(enums::domain::MITPFlags flags) const noexcept;

    [[nodiscard]] auto getBlockInfo(gsl::czstring<> disk) const noexcept -> std::optional<virDomainBlockInfo>;

    [[nodiscard]] auto getBlockIoTune(gsl::czstring<> disk, enums::domain::MITPFlags flags) const noexcept;

    [[nodiscard]] auto getBlockJobInfo(gsl::czstring<> disk, enums::domain::BlockJobInfoFlag flags) const noexcept;

    [[nodiscard]] Connection getConnect() const noexcept;

    [[nodiscard]] std::optional<virDomainControlInfo> getControlInfo() const noexcept;

    [[nodiscard]] auto getTotalCPUStats() const noexcept;

    [[nodiscard]] auto getCPUStats(unsigned start_cpu, unsigned ncpus) const noexcept;

    [[nodiscard]] auto getDiskErrors() const noexcept;

    [[nodiscard]] std::vector<DiskError> extractDiskErrors() const;

    //[[nodiscard]] CpuMap getEmulatorPinInfo(std::size_t maplen, ModificationImpactFlag flags) const noexcept;

    [[nodiscard]] auto getFSInfo() const noexcept;

    [[nodiscard]] std::vector<FSInfo> extractFSInfo() const;

    [[nodiscard]] auto getJobStats(enums::domain::GetJobStatsFlag flags) const noexcept;

    [[nodiscard]] std::optional<TypedParams> getGuestVcpus() const noexcept;

    [[nodiscard]] UniqueZstring getHostname() const noexcept;

    [[nodiscard]] std::string extractHostname() const noexcept;

    [[nodiscard]] unsigned getID() const noexcept;

    [[nodiscard]] auto getIOThreadInfo(enums::domain::ModificationImpactFlag flags) const noexcept;

    [[nodiscard]] auto extractIOThreadInfo(enums::domain::ModificationImpactFlag flags) const -> std::vector<heavy::IOThreadInfo>;

    [[nodiscard]] Info getInfo() const noexcept;

    [[nodiscard]] auto getInterfaceParameters(gsl::czstring<> device, enums::domain::MITPFlags flags) const noexcept;

    [[nodiscard]] std::optional<JobInfo> getJobInfo() const noexcept;

    [[nodiscard]] auto getLaunchSecurityInfo() const noexcept;

    [[nodiscard]] int getMaxVcpus() const noexcept;

    [[nodiscard]] auto getMemoryParameters(enums::domain::MITPFlags flags) const noexcept;

    [[nodiscard]] UniqueZstring getMetadata(enums::domain::MetadataType type, gsl::czstring<> ns,
                                            enums::domain::ModificationImpactFlag flags) const noexcept;

    [[nodiscard]] std::string extractMetadata(enums::domain::MetadataType type, gsl::czstring<> ns,
                                              enums::domain::ModificationImpactFlag flags) const;

    [[nodiscard]] gsl::czstring<> getName() const noexcept;

    [[nodiscard]] auto getNumaParameters(enums::domain::MITPFlags flags) const noexcept;

    [[nodiscard]] int getNumVcpus(enums::domain::VCpuFlag flags) const noexcept;

    [[nodiscard]] auto getSchedulerType() const noexcept -> std::pair<UniqueZstring, int>;

    [[nodiscard]] auto getSecurityLabel() const noexcept -> std::unique_ptr<virSecurityLabel>;

    [[nodiscard]] auto getSecurityLabelList() const noexcept;

    [[nodiscard]] auto extractSecurityLabelList() const -> std::vector<virSecurityLabel>;

    [[nodiscard]] auto getState() const noexcept -> StateWReason;

    [[nodiscard]] auto getTime() const noexcept;

    [[nodiscard]] auto getUUID() const;

    [[nodiscard]] bool isActive() const noexcept;

    [[nodiscard]] auto getUUIDString() const noexcept -> std::optional<std::array<char, VIR_UUID_STRING_BUFLEN>>;

    [[nodiscard]] auto extractUUIDString() const -> std::string;

    [[nodiscard]] auto getOSType() const;

    [[nodiscard]] unsigned long getMaxMemory() const noexcept;

    [[nodiscard]] auto getSchedulerParameters() const noexcept;

    [[nodiscard]] auto getSchedulerParameters(enums::domain::MITPFlags flags) const noexcept;

    [[nodiscard]] auto getPerfEvents(enums::domain::MITPFlags flags) const noexcept;

    [[nodiscard]] auto getVcpuPinInfo(enums::domain::VCpuFlag flags) -> std::optional<std::vector<unsigned char>>;

    [[nodiscard]] auto getVcpus() const noexcept;

    [[nodiscard]] UniqueZstring getXMLDesc(enums::domain::XMLFlags flag) const noexcept;

    [[nodiscard]] TFE hasManagedSaveImage() const noexcept;

    bool injectNMI() noexcept;

    [[nodiscard]] auto interfaceAddressesView(enums::domain::InterfaceAddressesSource source) const noexcept;

    [[nodiscard]] auto interfaceAddresses(enums::domain::InterfaceAddressesSource source) const -> std::vector<Interface>;

    [[nodiscard]] auto interfaceStats(gsl::czstring<> device) const noexcept -> std::optional<virDomainInterfaceStatsStruct>;

    [[nodiscard]] TFE isPersistent() const noexcept;

    [[nodiscard]] TFE isUpdated() const noexcept;

    bool PMSuspendForDuration(unsigned target, unsigned long long duration) noexcept;
    bool PMWakeup() noexcept;

    // [[nodiscard]] static int listGetStats(gsl::basic_zstring<Domain> doms, StatsType stats, virDomainStatsRecordPtr** retStats,
    // GetAllDomainStatsFlag flags);

    bool managedSave(enums::domain::SaveRestoreFlag flag) noexcept;

    bool managedSaveDefineXML(gsl::czstring<> dxml, enums::domain::SaveRestoreFlag flag) noexcept;

    [[nodiscard]] UniqueZstring managedSaveGetXMLDesc(enums::domain::SaveImageXMLFlag flag) const noexcept;

    [[nodiscard]] std::string managedSaveExtractXMLDesc(enums::domain::SaveImageXMLFlag flag) const noexcept;

    bool managedSaveRemove() noexcept;

    bool memoryPeek(unsigned long long start, gsl::span<unsigned char> buffer, enums::domain::MemoryFlag flag) const noexcept;

    auto memoryStats(unsigned int nr_stats) const noexcept;

    [[nodiscard]] Domain migrate(Connection dconn, enums::domain::MigrateFlag flags, gsl::czstring<> dname, gsl::czstring<> uri,
                                 unsigned long bandwidth) noexcept;

    [[nodiscard]] Domain migrate(Connection dconn, gsl::czstring<> dxml, enums::domain::MigrateFlag flags, gsl::czstring<> dname, gsl::czstring<> uri,
                                 unsigned long bandwidth) noexcept;

    [[nodiscard]] Domain migrate(Connection dconn, const TypedParams& params, enums::domain::MigrateFlag flags) noexcept;

    bool migrateToURI(gsl::czstring<> duri, enums::domain::MigrateFlag flags, gsl::czstring<> dname, unsigned long bandwidth) noexcept;

    bool migrateToURI(gsl::czstring<> dconnuri, gsl::czstring<> miguri, gsl::czstring<> dxml, enums::domain::MigrateFlag flags, gsl::czstring<> dname,
                      unsigned long bandwidth) noexcept;

    bool migrateToURI(gsl::czstring<> dconnuri, const TypedParams& params, enums::domain::MigrateFlag flags) noexcept;

    [[nodiscard]] auto migrateGetCompressionCache() const noexcept -> std::optional<unsigned long long>;

    [[nodiscard]] auto migrateGetMaxDowntime() const noexcept -> std::optional<unsigned long long>;

    [[nodiscard]] auto migrateGetMaxSpeed(unsigned int flag) const noexcept -> std::optional<unsigned long>;

    bool migrateSetCompressionCache(unsigned long long cacheSize) noexcept;

    bool migrateSetMaxDowntime(unsigned long long downtime) noexcept;

    bool migrateSetMaxSpeed(unsigned long bandwidth, unsigned int flag) noexcept;

    bool migrateStartPostCopy(unsigned int flag) noexcept;

    bool openChannel(gsl::czstring<> name, Stream& st, enums::domain::ChannelFlag flags) noexcept;

    bool openConsole(gsl::czstring<> dev_name, Stream& st, enums::domain::ConsoleFlag flags) noexcept;

    bool openGraphics(unsigned int idx, int fd, enums::domain::OpenGraphicsFlag flags) const noexcept;

    [[nodiscard]] int openGraphicsFD(unsigned int idx, enums::domain::OpenGraphicsFlag flags) const noexcept;

    bool pinEmulator(CpuMap cpumap, enums::domain::ModificationImpactFlag flags) noexcept;

    bool pinIOThread(unsigned int iothread_id, CpuMap cpumap, enums::domain::ModificationImpactFlag flags) noexcept;

    bool pinVcpu(unsigned int vcpu, CpuMap cpumap) noexcept;

    bool pinVcpuFlags(unsigned int vcpu, CpuMap cpumap, enums::domain::ModificationImpactFlag flags) noexcept;

    bool sendKey(enums::domain::KeycodeSet codeset, unsigned int holdtime, gsl::span<const unsigned int> keycodes) noexcept;

    bool sendProcessSignal(long long pid_value, enums::domain::ProcessSignal signum) noexcept;

    bool setMaxMemory(unsigned long);

    bool setMemory(unsigned long);

    bool setMemoryStatsPeriod(int period, enums::domain::MemoryModFlag flags) noexcept;

    bool reboot(enums::domain::ShutdownFlag flags);
    bool reboot();

    bool reset();

    bool rename(gsl::czstring<>);

    bool resume() noexcept;

    bool save(gsl::czstring<> to) noexcept;

    bool save(gsl::czstring<> to, gsl::czstring<> dxml, enums::domain::SaveRestoreFlag flags) noexcept;

    UniqueZstring screenshot(Stream& stream, unsigned int screen) const noexcept;

    bool setAutoStart(bool);

    bool setBlkioParameters(TypedParams params, enums::domain::ModificationImpactFlag flags) noexcept;

    bool setBlockIoTune(gsl::czstring<> disk, TypedParams params, enums::domain::ModificationImpactFlag flags) noexcept;

    bool setBlockThreshold(gsl::czstring<> dev, unsigned long long threshold) noexcept;

    bool setGuestVcpus(gsl::czstring<> cpumap, bool state) noexcept; // https://libvirt.org/html/libvirt-libvirt-domain.html#virDomainSetGuestVcpus

    bool setIOThreadParams(unsigned int iothread_id, TypedParams params, enums::domain::MITPFlags flags) noexcept;

    bool setInterfaceParameters(gsl::czstring<> device, TypedParams params, enums::domain::ModificationImpactFlag flags) noexcept;

    bool setLifecycleAction(enums::domain::Lifecycle type, enums::domain::LifecycleAction action,
                            enums::domain::ModificationImpactFlag flags) noexcept;

    bool setMemoryFlags(unsigned long memory, enums::domain::MemoryModFlag flags) noexcept;

    bool setMemoryParameters(TypedParams params, enums::domain::ModificationImpactFlag flags) noexcept;

    bool setNumaParameters(TypedParams params, enums::domain::ModificationImpactFlag flags) noexcept;

    bool setPerfEvents(TypedParams params, enums::domain::ModificationImpactFlag flags) noexcept;

    bool setSchedulerParameters(TypedParams params) noexcept;

    bool setSchedulerParameters(TypedParams params, enums::domain::ModificationImpactFlag flags) noexcept;

    bool setMetadata(enums::domain::MetadataType type, gsl::czstring<> metadata, gsl::czstring<> key, gsl::czstring<> uri,
                     enums::domain::ModificationImpactFlag flags) noexcept;

    bool setTime(long long seconds, unsigned int nseconds, enums::domain::SetTimeFlag flags) noexcept;

    bool setUserPassword(gsl::czstring<> user, gsl::czstring<> password, enums::domain::SetUserPasswordFlag flags) noexcept;

    bool setVcpu(gsl::czstring<> vcpumap, bool state, enums::domain::ModificationImpactFlag flags) noexcept;

    bool setVcpus(unsigned int nvcpus) noexcept;

    bool setVcpus(unsigned int nvcpus, enums::domain::VCpuFlag flags) noexcept;

    bool shutdown() noexcept;

    bool shutdown(enums::domain::ShutdownFlag flag) noexcept;

    bool suspend() noexcept;

    bool undefine() noexcept;

    bool undefine(enums::domain::UndefineFlag) noexcept;

    bool updateDeviceFlags(gsl::czstring<> xml, enums::domain::DeviceModifyFlag flags) noexcept;

    [[nodiscard]] static Domain createXML(Connection&, gsl::czstring<> xml, enums::domain::CreateFlag flags);
    [[nodiscard]] static Domain createXML(Connection&, gsl::czstring<> xml);

    // [[nodiscard]] static Domain defineXML();
};

class Domain::StatsRecord {
    friend Connection;

    Domain dom;
    std::vector<TypedParameter> params{};

    explicit StatsRecord(const virDomainStatsRecord&) noexcept;

  public:
};

class Domain::StateWReason
    : public std::variant<enums::domain::state_reason::NoState, enums::domain::state_reason::Running, enums::domain::state_reason::Blocked,
                          enums::domain::state_reason::Paused, enums::domain::state_reason::Shutdown, enums::domain::state_reason::Shutoff,
                          enums::domain::state_reason::Crashed, enums::domain::state_reason::PMSuspended> {
    constexpr enums::domain::State state() const noexcept { return enums::domain::State(EHTag{}, this->index()); }
};

struct alignas(alignof(virDomainDiskError)) Domain::DiskError {
    enum class Code : decltype(virDomainDiskError::error) {
        NONE = VIR_DOMAIN_DISK_ERROR_NONE,         /* no error */
        UNSPEC = VIR_DOMAIN_DISK_ERROR_UNSPEC,     /* unspecified I/O error */
        NO_SPACE = VIR_DOMAIN_DISK_ERROR_NO_SPACE, /* no space left on the device */
    };

    UniqueZstring disk;
    Code error;
};

struct Domain::FSInfo {
    std::string mountpoint;             /* path to mount point */
    std::string name;                   /* device name in the guest (e.g. "sda1") */
    std::string fstype;                 /* filesystem type */
    std::vector<std::string> dev_alias; /* vector of disk device aliases */

    FSInfo(virDomainFSInfo* from) noexcept
        : mountpoint(from->mountpoint), name(from->name), fstype(from->fstype), dev_alias(from->devAlias, from->devAlias + from->ndevAlias) {
        virDomainFSInfoFree(from);
    }
};

class Domain::IPAddress {
    friend Domain;
    friend Interface;
    IPAddress(virDomainIPAddressPtr ptr) : type(Type{ptr->type}), addr(ptr->addr), prefix(ptr->prefix) {}

  public:
    enum class Type : int {
        IPV4 = VIR_IP_ADDR_TYPE_IPV4,
        IPV6 = VIR_IP_ADDR_TYPE_IPV6,
    };
    IPAddress() noexcept = default;
    IPAddress(const virDomainIPAddress& ref) : type(Type{ref.type}), addr(ref.addr), prefix(ref.prefix) {}
    IPAddress(Type type, std::string addr, uint8_t prefix) noexcept : type(type), addr(std::move(addr)), prefix(prefix) {}
    Type type;
    std::string addr;
    uint8_t prefix;
};

class Domain::IPAddressView : private virDomainIPAddress {
    friend InterfaceView;
    using Base = virDomainIPAddress;

  public:
    [[nodiscard]] constexpr IPAddress::Type type() const noexcept { return IPAddress::Type{Base::type}; }
    [[nodiscard]] constexpr gsl::czstring<> addr() const noexcept { return Base::addr; }
    [[nodiscard]] constexpr uint8_t prefix() const noexcept { return Base::prefix; }

    [[nodiscard]] operator IPAddress() const noexcept { return {type(), addr(), prefix()}; };
};

class Domain::Interface {
    friend Domain;

  public:
    Interface(virDomainInterfacePtr ptr) : name(ptr->name), hwaddr(ptr->hwaddr), addrs(ptr->addrs, ptr->addrs + ptr->naddrs) {}
    std::string name;
    std::string hwaddr;
    std::vector<IPAddress> addrs;
};

class Domain::InterfaceView : private virDomainInterface {
    using Base = virDomainInterface;

  public:
    ~InterfaceView() noexcept { virDomainInterfaceFree(this); }
    [[nodiscard]] constexpr gsl::czstring<> name() const noexcept { return Base::name; }
    [[nodiscard]] constexpr gsl::czstring<> hwaddr() const noexcept { return Base::hwaddr; }
    [[nodiscard]] constexpr gsl::span<IPAddressView> addrs() const noexcept { return {static_cast<IPAddressView*>(Base::addrs), Base::naddrs}; }
};

struct alignas(alignof(virDomainJobInfo)) Domain::JobInfo : public virDomainJobInfo {
    [[nodiscard]] constexpr enums::domain::JobType type() const noexcept { return enums::domain::JobType{virDomainJobInfo::type}; }
};

// concept Domain::IOThreadInfo

class alignas(alignof(virDomainIOThreadInfo)) Domain::light::IOThreadInfo : private virDomainIOThreadInfo {
    friend Domain;

    using Base = virDomainIOThreadInfo;

  public:
    inline ~IOThreadInfo() noexcept { virDomainIOThreadInfoFree(this); }
    constexpr unsigned iothread_id() const noexcept { return Base::iothread_id; }
    constexpr unsigned& iothread_id() noexcept { return Base::iothread_id; }
    constexpr CpuMap cpumap() const noexcept { return {Base::cpumap, Base::cpumaplen}; }
    constexpr CpuMap cpumap() noexcept { return {Base::cpumap, Base::cpumaplen}; }
};

class Domain::heavy::IOThreadInfo {
    friend Domain;

    unsigned m_iothread_id{};
    std::vector<unsigned char> m_cpumap{};

    IOThreadInfo(const virDomainIOThreadInfo& ref) noexcept
        : m_iothread_id(ref.iothread_id), m_cpumap(ref.cpumap, ref.cpumap + ref.cpumaplen) {} // C++2aTODO make constexpr
  public:
    IOThreadInfo(virDomainIOThreadInfo* ptr) noexcept
        : m_iothread_id(ptr->iothread_id), m_cpumap(ptr->cpumap, ptr->cpumap + ptr->cpumaplen) {} // C++2aTODO make constexpr
    inline ~IOThreadInfo() = default;
    constexpr unsigned iothread_id() const noexcept { return m_iothread_id; }
    constexpr unsigned& iothread_id() noexcept { return m_iothread_id; }
    gsl::span<const unsigned char> cpumap() const noexcept { return {m_cpumap.data(), static_cast<long>(m_cpumap.size())}; }
    CpuMap cpumap() noexcept { return {m_cpumap.data(), static_cast<int>(m_cpumap.size())}; }
};

} // namespace virt

#include "impl/Domain.hpp"

#endif