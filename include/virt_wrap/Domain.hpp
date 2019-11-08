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
#include <libvirt/libvirt-domain.h>
#include "CpuMap.hpp"
#include "GFlags.hpp"
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
    struct Stats {
        enum class Types {
            STATE = VIR_DOMAIN_STATS_STATE,         // domain state
            CPU_TOTAL = VIR_DOMAIN_STATS_CPU_TOTAL, // domain CPU info
            BALLOON = VIR_DOMAIN_STATS_BALLOON,     // domain balloon info
            VCPU = VIR_DOMAIN_STATS_VCPU,           // domain virtual CPU info
            INTERFACE = VIR_DOMAIN_STATS_INTERFACE, // domain interfaces info
            BLOCK = VIR_DOMAIN_STATS_BLOCK,         // domain block info
            PERF = VIR_DOMAIN_STATS_PERF,           // domain perf event info
            IOTHREAD = VIR_DOMAIN_STATS_IOTHREAD,   // iothread poll info
        };

        class Record;
    };
    class BlockCommitFlag;
    class BlockCopyFlag;
    class BlockJobAbortFlag;
    class BlockJobInfoFlag;
    class BlockJobSetSpeedFlag;
    class BlockPullFlag;
    class BlockRebaseFlag;
    class BlockResizeFlag;
    class ChannelFlag;
    class ConsoleFlag;

    struct CoreDump {
        enum class Flag {
            CRASH = VIR_DUMP_CRASH,               /* crash after dump */
            LIVE = VIR_DUMP_LIVE,                 /* live dump */
            BYPASS_CACHE = VIR_DUMP_BYPASS_CACHE, /* avoid file system cache pollution */
            RESET = VIR_DUMP_RESET,               /* reset domain after dump finishes */
            MEMORY_ONLY = VIR_DUMP_MEMORY_ONLY,   /* use dump-guest-memory */
        };
        enum class Format {
            RAW = VIR_DOMAIN_CORE_DUMP_FORMAT_RAW,                   /* dump guest memory in raw format */
            KDUMP_ZLIB = VIR_DOMAIN_CORE_DUMP_FORMAT_KDUMP_ZLIB,     /* kdump-compressed format, with zlib compression */
            KDUMP_LZO = VIR_DOMAIN_CORE_DUMP_FORMAT_KDUMP_LZO,       /* kdump-compressed format, with lzo compression */
            KDUMP_SNAPPY = VIR_DOMAIN_CORE_DUMP_FORMAT_KDUMP_SNAPPY, /* kdump-compressed format, with snappy compression */
        };
    };

    class CreateFlag;
    class DestroyFlag;
    class DeviceModifyFlag;
    class GetJobStatsFlag;

    enum class GetAllDomainStatsFlag : unsigned {
        ACTIVE = VIR_CONNECT_GET_ALL_DOMAINS_STATS_ACTIVE,
        INACTIVE = VIR_CONNECT_GET_ALL_DOMAINS_STATS_INACTIVE,

        PERSISTENT = VIR_CONNECT_GET_ALL_DOMAINS_STATS_PERSISTENT,
        TRANSIENT = VIR_CONNECT_GET_ALL_DOMAINS_STATS_TRANSIENT,

        RUNNING = VIR_CONNECT_GET_ALL_DOMAINS_STATS_RUNNING,
        PAUSED = VIR_CONNECT_GET_ALL_DOMAINS_STATS_PAUSED,
        SHUTOFF = VIR_CONNECT_GET_ALL_DOMAINS_STATS_SHUTOFF,
        OTHER = VIR_CONNECT_GET_ALL_DOMAINS_STATS_OTHER,

        NOWAIT = VIR_CONNECT_GET_ALL_DOMAINS_STATS_NOWAIT,       /* report statistics that can be obtained
                                                                immediately without any blocking */
        BACKING = VIR_CONNECT_GET_ALL_DOMAINS_STATS_BACKING,     /* include backing chain for block stats */
        STATS = VIR_CONNECT_GET_ALL_DOMAINS_STATS_ENFORCE_STATS, /* enforce requested stats */
    };
    enum class InterfaceAddressesSource : unsigned {
        LEASE = VIR_DOMAIN_INTERFACE_ADDRESSES_SRC_LEASE, /* Parse DHCP lease file */
        AGENT = VIR_DOMAIN_INTERFACE_ADDRESSES_SRC_AGENT, /* Query qemu guest agent */
        ARP = VIR_DOMAIN_INTERFACE_ADDRESSES_SRC_ARP,     /* Query ARP tables */
    };
    enum class JobType {
        NONE = VIR_DOMAIN_JOB_NONE,           /* No job is active */
        BOUNDED = VIR_DOMAIN_JOB_BOUNDED,     /* Job with a finite completion time */
        UNBOUNDED = VIR_DOMAIN_JOB_UNBOUNDED, /* Job without a finite completion time */
        COMPLETED = VIR_DOMAIN_JOB_COMPLETED, /* Job has finished, but isn't cleaned up */
        FAILED = VIR_DOMAIN_JOB_FAILED,       /* Job hit error, but isn't cleaned up */
        CANCELLED = VIR_DOMAIN_JOB_CANCELLED, /* Job was aborted, but isn't cleaned up */
    };

    class KeycodeSet;
    class Lifecycle;
    class LifecycleAction;
    class MemoryModFlag;
    class OpenGraphicsFlag;

    enum class SaveImageXMLFlag : unsigned {
        SECURE = VIR_DOMAIN_XML_SECURE, /* dump security sensitive information too */
    };

    class SaveRestoreFlag;
    class ShutdownFlag;

    struct StateReason {
        enum class NoState {
            UNKNOWN = VIR_DOMAIN_NOSTATE_UNKNOWN, /* the reason is unknown */
        };
        enum class Running {
            UNKNOWN = VIR_DOMAIN_RUNNING_UNKNOWN,                       /* the reason is unknown */
            BOOTED = VIR_DOMAIN_RUNNING_BOOTED,                         /* normal startup from boot */
            MIGRATED = VIR_DOMAIN_RUNNING_MIGRATED,                     /* migrated from another host */
            RESTORED = VIR_DOMAIN_RUNNING_RESTORED,                     /* restored from a state file */
            FROM_SNAPSHOT = VIR_DOMAIN_RUNNING_FROM_SNAPSHOT,           /* restored from snapshot */
            UNPAUSED = VIR_DOMAIN_RUNNING_UNPAUSED,                     /* returned from paused state */
            MIGRATION_CANCELED = VIR_DOMAIN_RUNNING_MIGRATION_CANCELED, /* returned from migration */
            SAVE_CANCELED = VIR_DOMAIN_RUNNING_SAVE_CANCELED,           /* returned from failed save process */
            WAKEUP = VIR_DOMAIN_RUNNING_WAKEUP,                         /* returned from pmsuspended due to wakeup event */
            CRASHED = VIR_DOMAIN_RUNNING_CRASHED,                       /* resumed from crashed */
            POSTCOPY = VIR_DOMAIN_RUNNING_POSTCOPY,                     /* running in post-copy migration mode */
        };
        enum class Blocked {
            UNKNOWN = VIR_DOMAIN_BLOCKED_UNKNOWN, /* the reason is unknown */
        };
        enum class Paused {
            UNKNOWN = VIR_DOMAIN_PAUSED_UNKNOWN,                 /* the reason is unknown */
            USER = VIR_DOMAIN_PAUSED_USER,                       /* paused on user request */
            MIGRATION = VIR_DOMAIN_PAUSED_MIGRATION,             /* paused for offline migration */
            SAVE = VIR_DOMAIN_PAUSED_SAVE,                       /* paused for save */
            DUMP = VIR_DOMAIN_PAUSED_DUMP,                       /* paused for offline core dump */
            IOERROR = VIR_DOMAIN_PAUSED_IOERROR,                 /* paused due to a disk I/O error */
            WATCHDOG = VIR_DOMAIN_PAUSED_WATCHDOG,               /* paused due to a watchdog event */
            FROM_SNAPSHOT = VIR_DOMAIN_PAUSED_FROM_SNAPSHOT,     /* paused after restoring from snapshot */
            SHUTTING_DOWN = VIR_DOMAIN_PAUSED_SHUTTING_DOWN,     /* paused during shutdown process */
            SNAPSHOT = VIR_DOMAIN_PAUSED_SNAPSHOT,               /* paused while creating a snapshot */
            CRASHED = VIR_DOMAIN_PAUSED_CRASHED,                 /* paused due to a guest crash */
            STARTING_UP = VIR_DOMAIN_PAUSED_STARTING_UP,         /* the domain is being started */
            POSTCOPY = VIR_DOMAIN_PAUSED_POSTCOPY,               /* paused for post-copy migration */
            POSTCOPY_FAILED = VIR_DOMAIN_PAUSED_POSTCOPY_FAILED, /* paused after failed post-copy */
        };
        enum class Shutdown {
            UNKNOWN = VIR_DOMAIN_SHUTDOWN_UNKNOWN, /* the reason is unknown */
            USER = VIR_DOMAIN_SHUTDOWN_USER,       /* shutting down on user request */
        };
        enum class Shutoff {
            UNKNOWN = VIR_DOMAIN_SHUTOFF_UNKNOWN,             /* the reason is unknown */
            SHUTDOWN = VIR_DOMAIN_SHUTOFF_SHUTDOWN,           /* normal shutdown */
            DESTROYED = VIR_DOMAIN_SHUTOFF_DESTROYED,         /* forced poweroff */
            CRASHED = VIR_DOMAIN_SHUTOFF_CRASHED,             /* domain crashed */
            MIGRATED = VIR_DOMAIN_SHUTOFF_MIGRATED,           /* migrated to another host */
            SAVED = VIR_DOMAIN_SHUTOFF_SAVED,                 /* saved to a file */
            FAILED = VIR_DOMAIN_SHUTOFF_FAILED,               /* domain failed to start */
            FROM_SNAPSHOT = VIR_DOMAIN_SHUTOFF_FROM_SNAPSHOT, /* restored from a snapshot which was taken while domain was shutoff */
            DAEMON = VIR_DOMAIN_SHUTOFF_DAEMON,               /* daemon decides to kill domain during reconnection processing */
        };
        enum class Crashed {
            UNKNOWN = VIR_DOMAIN_CRASHED_UNKNOWN,   /* crashed for unknown reason */
            PANICKED = VIR_DOMAIN_CRASHED_PANICKED, /* domain panicked */
        };
        enum class PMSuspended {
            UNKNOWN = VIR_DOMAIN_PMSUSPENDED_UNKNOWN, /* the reason is unknown */
        };
    };
    struct StateWReason;
    enum class StatsType {
        STATE = VIR_DOMAIN_STATS_STATE,         /* return domain state */
        TOTAL = VIR_DOMAIN_STATS_CPU_TOTAL,     /* return domain CPU info */
        BALLOON = VIR_DOMAIN_STATS_BALLOON,     /* return domain balloon info */
        VCPU = VIR_DOMAIN_STATS_VCPU,           /* return domain virtual CPU info */
        INTERFACE = VIR_DOMAIN_STATS_INTERFACE, /* return domain interfaces info */
        BLOCK = VIR_DOMAIN_STATS_BLOCK,         /* return domain block info */
        PERF = VIR_DOMAIN_STATS_PERF,           /* return domain perf event info */
        IOTHREAD = VIR_DOMAIN_STATS_IOTHREAD,   /* return iothread poll info */
    };

    enum class MemoryFlag {
        VIRTUAL = VIR_MEMORY_VIRTUAL,   /* addresses are virtual addresses */
        PHYSICAL = VIR_MEMORY_PHYSICAL, /* addresses are physical addresses */
    };

    enum class MemoryStatTag {
        /* The total amount of data read from swap space (in kB). */
        SWAP_IN = VIR_DOMAIN_MEMORY_STAT_SWAP_IN,
        /* The total amount of memory written out to swap space (in kB). */
        SWAP_OUT = VIR_DOMAIN_MEMORY_STAT_SWAP_OUT,

        /*
         * Page faults occur when a process makes a valid access to virtual memory
         * that is not available.  When servicing the page fault, if disk IO is
         * required, it is considered a major fault.  If not, it is a minor fault.
         * These are expressed as the number of faults that have occurred.
         */
        MAJOR_FAULT = VIR_DOMAIN_MEMORY_STAT_MAJOR_FAULT,
        MINOR_FAULT = VIR_DOMAIN_MEMORY_STAT_MINOR_FAULT,

        /*
         * The amount of memory left completely unused by the system.  Memory that
         * is available but used for reclaimable caches should NOT be reported as
         * free.  This value is expressed in kB.
         */
        UNUSED = VIR_DOMAIN_MEMORY_STAT_UNUSED,

        /*
         * The total amount of usable memory as seen by the domain.  This value
         * may be less than the amount of memory assigned to the domain if a
         * balloon driver is in use or if the guest OS does not initialize all
         * assigned pages.  This value is expressed in kB.
         */
        AVAILABLE = VIR_DOMAIN_MEMORY_STAT_AVAILABLE,

        /* Current balloon value (in KB). */
        ACTUAL_BALLOON = VIR_DOMAIN_MEMORY_STAT_ACTUAL_BALLOON,

        /* Resident Set Size of the process running the domain. This value
         * is in kB */
        RSS = VIR_DOMAIN_MEMORY_STAT_RSS,

        /*
         * How much the balloon can be inflated without pushing the guest system
         * to swap, corresponds to 'Available' in /proc/meminfo
         */
        USABLE = VIR_DOMAIN_MEMORY_STAT_USABLE,

        /* Timestamp of the last update of statistics, in seconds. */
        UPDATE = VIR_DOMAIN_MEMORY_STAT_LAST_UPDATE,

        /*
         * The amount of memory, that can be quickly reclaimed without
         * additional I/O (in kB). Typically these pages are used for caching files
         * from disk.
         */
        DISK_CACHES = VIR_DOMAIN_MEMORY_STAT_DISK_CACHES,

        /*
         * The number of statistics supported by this version of the interface.
         * To add new statistics, add them to the enum and increase this value.
         */
        NR = VIR_DOMAIN_MEMORY_STAT_NR,
    };

    class MetadataType;
    class MigrateFlag;
    class ModificationImpactFlag;
    class ProcessSignal;

    enum class SetTimeFlag {
        SYNC = VIR_DOMAIN_TIME_SYNC, /* Re-sync domain time from domain's RTC */
    };
    enum class SetUserPasswordFlag {
        ENCRYPTED = VIR_DOMAIN_PASSWORD_ENCRYPTED, /* the password is already encrypted */
    };
    enum class VCpuFlag {
        /* See ModificationImpactFlag for these flags.  */
        CURRENT = VIR_DOMAIN_VCPU_CURRENT,
        LIVE = VIR_DOMAIN_VCPU_LIVE,
        CONFIG = VIR_DOMAIN_VCPU_CONFIG,

        /* Additionally, these flags may be bitwise-OR'd in.  */
        MAXIMUM = VIR_DOMAIN_VCPU_MAXIMUM,           /* Max rather than current count */
        GUEST = VIR_DOMAIN_VCPU_GUEST,               /* Modify state of the cpu in the guest */
        HOTPLUGGABLE = VIR_DOMAIN_VCPU_HOTPLUGGABLE, /* Make vcpus added hot(un)pluggable */
    };

    class XmlFlag;
    class State;
    class UndefineFlag;

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

    using MITPFlags = EnumSetTie<ModificationImpactFlag, TypedParameterFlag>;

    constexpr inline explicit Domain(virDomainPtr ptr = nullptr) noexcept;

    Domain(const Domain&) = delete;

    constexpr inline Domain(Domain&&) noexcept;

    Domain& operator=(const Domain&) = delete;

    inline Domain& operator=(Domain&&) noexcept;

    inline ~Domain() noexcept;

    constexpr inline explicit operator bool() const noexcept;

    bool abortJob() noexcept;

    bool addIOThread(unsigned int iothread_id, ModificationImpactFlag flags) noexcept;

    bool attachDevice(gsl::czstring<> xml) noexcept;

    bool attachDevice(gsl::czstring<> xml, DeviceModifyFlag flags) noexcept;

    bool blockCommit(gsl::czstring<> disk, gsl::czstring<> base, gsl::czstring<> top, unsigned long bandwidth, BlockCommitFlag flags) noexcept;

    bool blockCopy(gsl::czstring<> disk, gsl::czstring<> destxml, const TypedParams& params, BlockCopyFlag flags) noexcept;

    bool blockJobAbort(gsl::czstring<> disk, BlockJobAbortFlag flags) noexcept;

    bool blockJobSetSpeed(gsl::czstring<> disk, unsigned long bandwidth, BlockJobSetSpeedFlag flags) noexcept;

    bool blockPeek(gsl::czstring<> disk, unsigned long long offset, gsl::span<std::byte> buffer) const noexcept;

    bool blockPull(gsl::czstring<> disk, unsigned long bandwidth, BlockPullFlag flags) noexcept;

    bool blockRebase(gsl::czstring<> disk, gsl::czstring<> base, unsigned long bandwidth, BlockRebaseFlag flags);

    bool blockResize(gsl::czstring<> disk, unsigned long long size, BlockResizeFlag flags) noexcept;

    auto blockStats(gsl::czstring<> disk, size_t size) const noexcept;

    auto blockStatsFlags(gsl::czstring<> disk, TypedParameterFlag flags) const noexcept;

    bool create() noexcept;

    bool create(CreateFlag flag) noexcept;

    // createWithFiles() // Left out

    bool coreDump(std::filesystem::path to, CoreDump::Flag flags) const noexcept;

    bool coreDump(std::filesystem::path to, CoreDump::Format format, CoreDump::Flag flags) const noexcept;

    bool delIOThread(unsigned int iothread_id, ModificationImpactFlag flags) noexcept;

    bool destroy() noexcept;

    bool destroy(DestroyFlag flag) noexcept;

    bool detachDevice(gsl::czstring<> xml) noexcept;

    bool detachDevice(gsl::czstring<> xml, DeviceModifyFlag flag) noexcept;

    bool detachDeviceAlias(gsl::czstring<> alias, DeviceModifyFlag flag) noexcept;

    int fsFreeze(gsl::span<gsl::czstring<>> mountpoints) noexcept;

    int fsThaw(gsl::span<gsl::czstring<>> mountpoints) noexcept;

    bool fsTrim(gsl::czstring<> mountpoint, unsigned long long minimum) noexcept;

    [[nodiscard]] bool getAutostart() const noexcept;

    [[nodiscard]] auto getBlkioParameters(MITPFlags flags) const noexcept;

    [[nodiscard]] auto getBlockInfo(gsl::czstring<> disk) const noexcept -> std::optional<virDomainBlockInfo>;

    [[nodiscard]] auto getBlockIoTune(gsl::czstring<> disk, MITPFlags flags) const noexcept;

    [[nodiscard]] auto getBlockJobInfo(gsl::czstring<> disk, BlockJobInfoFlag flags) const noexcept;

    [[nodiscard]] Connection getConnect() const noexcept;

    [[nodiscard]] std::optional<virDomainControlInfo> getControlInfo() const noexcept;

    [[nodiscard]] auto getTotalCPUStats() const noexcept;

    [[nodiscard]] auto getCPUStats(unsigned start_cpu, unsigned ncpus) const noexcept;

    [[nodiscard]] auto getDiskErrors() const noexcept;

    [[nodiscard]] std::vector<DiskError> extractDiskErrors() const;

    //[[nodiscard]] CpuMap getEmulatorPinInfo(std::size_t maplen, ModificationImpactFlag flags) const noexcept;

    [[nodiscard]] auto getFSInfo() const noexcept;

    [[nodiscard]] std::vector<FSInfo> extractFSInfo() const;

    [[nodiscard]] auto getJobStats(GetJobStatsFlag flags) const noexcept;

    [[nodiscard]] std::optional<TypedParams> getGuestVcpus() const noexcept;

    [[nodiscard]] UniqueZstring getHostname() const noexcept;

    [[nodiscard]] std::string extractHostname() const noexcept;

    [[nodiscard]] unsigned getID() const noexcept;

    [[nodiscard]] auto getIOThreadInfo(ModificationImpactFlag flags) const noexcept;

    [[nodiscard]] auto extractIOThreadInfo(ModificationImpactFlag flags) const -> std::vector<heavy::IOThreadInfo>;

    [[nodiscard]] Info getInfo() const noexcept;

    [[nodiscard]] auto getInterfaceParameters(gsl::czstring<> device, MITPFlags flags) const noexcept;

    [[nodiscard]] std::optional<JobInfo> getJobInfo() const noexcept;

    [[nodiscard]] auto getLaunchSecurityInfo() const noexcept;

    [[nodiscard]] int getMaxVcpus() const noexcept;

    [[nodiscard]] auto getMemoryParameters(MITPFlags flags) const noexcept;

    [[nodiscard]] UniqueZstring getMetadata(MetadataType type, gsl::czstring<> ns, ModificationImpactFlag flags) const noexcept;

    [[nodiscard]] std::string extractMetadata(MetadataType type, gsl::czstring<> ns, ModificationImpactFlag flags) const;

    [[nodiscard]] gsl::czstring<> getName() const noexcept;

    [[nodiscard]] auto getNumaParameters(MITPFlags flags) const noexcept;

    [[nodiscard]] int getNumVcpus(VCpuFlag flags) const noexcept;

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

    [[nodiscard]] auto getSchedulerParameters(MITPFlags flags) const noexcept;

    [[nodiscard]] auto getPerfEvents(MITPFlags flags) const noexcept;

    [[nodiscard]] auto getVcpuPinInfo(VCpuFlag flags) -> std::optional<std::vector<unsigned char>>;

    [[nodiscard]] auto getVcpus() const noexcept;

    [[nodiscard]] gsl::czstring<> getXMLDesc(XmlFlag flag) const noexcept;

    [[nodiscard]] TFE hasManagedSaveImage() const noexcept;

    bool injectNMI() noexcept;

    [[nodiscard]] auto interfaceAddressesView(InterfaceAddressesSource source) const noexcept;

    [[nodiscard]] auto interfaceAddresses(InterfaceAddressesSource source) const -> std::vector<Interface>;

    [[nodiscard]] auto interfaceStats(gsl::czstring<> device) const noexcept -> std::optional<virDomainInterfaceStatsStruct>;

    [[nodiscard]] TFE isPersistent() const noexcept;

    [[nodiscard]] TFE isUpdated() const noexcept;

    bool PMSuspendForDuration(unsigned target, unsigned long long duration) noexcept;
    bool PMWakeup() noexcept;

    // [[nodiscard]] static int listGetStats(gsl::basic_zstring<Domain> doms, StatsType stats, virDomainStatsRecordPtr** retStats,
    // GetAllDomainStatsFlag flags);

    bool managedSave(SaveRestoreFlag flag) noexcept;

    bool managedSaveDefineXML(gsl::czstring<> dxml, SaveRestoreFlag flag) noexcept;

    [[nodiscard]] UniqueZstring managedSaveGetXMLDesc(SaveImageXMLFlag flag) const noexcept;

    [[nodiscard]] std::string managedSaveExtractXMLDesc(SaveImageXMLFlag flag) const noexcept;

    bool managedSaveRemove() noexcept;

    bool memoryPeek(unsigned long long start, gsl::span<unsigned char> buffer, MemoryFlag flag) const noexcept;

    auto memoryStats(unsigned int nr_stats) const noexcept;

    [[nodiscard]] Domain migrate(Connection dconn, MigrateFlag flags, gsl::czstring<> dname, gsl::czstring<> uri, unsigned long bandwidth) noexcept;

    [[nodiscard]] Domain migrate(Connection dconn, gsl::czstring<> dxml, MigrateFlag flags, gsl::czstring<> dname, gsl::czstring<> uri,
                                 unsigned long bandwidth) noexcept;

    [[nodiscard]] Domain migrate(Connection dconn, const TypedParams& params, MigrateFlag flags) noexcept;

    bool migrateToURI(gsl::czstring<> duri, MigrateFlag flags, gsl::czstring<> dname, unsigned long bandwidth) noexcept;

    bool migrateToURI(gsl::czstring<> dconnuri, gsl::czstring<> miguri, gsl::czstring<> dxml, MigrateFlag flags, gsl::czstring<> dname,
                      unsigned long bandwidth) noexcept;

    bool migrateToURI(gsl::czstring<> dconnuri, const TypedParams& params, MigrateFlag flags) noexcept;

    [[nodiscard]] auto migrateGetCompressionCache() const noexcept -> std::optional<unsigned long long>;

    [[nodiscard]] auto migrateGetMaxDowntime() const noexcept -> std::optional<unsigned long long>;

    [[nodiscard]] auto migrateGetMaxSpeed(unsigned int flag) const noexcept -> std::optional<unsigned long>;

    bool migrateSetCompressionCache(unsigned long long cacheSize) noexcept;

    bool migrateSetMaxDowntime(unsigned long long downtime) noexcept;

    bool migrateSetMaxSpeed(unsigned long bandwidth, unsigned int flag) noexcept;

    bool migrateStartPostCopy(unsigned int flag) noexcept;

    bool openChannel(gsl::czstring<> name, Stream& st, ChannelFlag flags) noexcept;

    bool openConsole(gsl::czstring<> dev_name, Stream& st, ConsoleFlag flags) noexcept;

    bool openGraphics(unsigned int idx, int fd, OpenGraphicsFlag flags) const noexcept;

    [[nodiscard]] int openGraphicsFD(unsigned int idx, OpenGraphicsFlag flags) const noexcept;

    bool pinEmulator(CpuMap cpumap, ModificationImpactFlag flags) noexcept;

    bool pinIOThread(unsigned int iothread_id, CpuMap cpumap, ModificationImpactFlag flags) noexcept;

    bool pinVcpu(unsigned int vcpu, CpuMap cpumap) noexcept;

    bool pinVcpuFlags(unsigned int vcpu, CpuMap cpumap, ModificationImpactFlag flags) noexcept;

    bool sendKey(KeycodeSet codeset, unsigned int holdtime, gsl::span<const unsigned int> keycodes) noexcept;

    bool sendProcessSignal(long long pid_value, ProcessSignal signum) noexcept;

    bool setMaxMemory(unsigned long);

    bool setMemory(unsigned long);

    bool setMemoryStatsPeriod(int period, MemoryModFlag flags) noexcept;

    bool reboot(ShutdownFlag flags);
    bool reboot();

    bool reset();

    bool rename(gsl::czstring<>);

    bool resume() noexcept;

    bool save(gsl::czstring<> to) noexcept;

    bool save(gsl::czstring<> to, gsl::czstring<> dxml, SaveRestoreFlag flags) noexcept;

    UniqueZstring screenshot(Stream& stream, unsigned int screen) const noexcept;

    bool setAutoStart(bool);

    bool setBlkioParameters(TypedParams params, ModificationImpactFlag flags) noexcept;

    bool setBlockIoTune(gsl::czstring<> disk, TypedParams params, ModificationImpactFlag flags) noexcept;

    bool setBlockThreshold(gsl::czstring<> dev, unsigned long long threshold) noexcept;

    bool setGuestVcpus(gsl::czstring<> cpumap, bool state) noexcept; // https://libvirt.org/html/libvirt-libvirt-domain.html#virDomainSetGuestVcpus

    bool setIOThreadParams(unsigned int iothread_id, TypedParams params, MITPFlags flags) noexcept;

    bool setInterfaceParameters(gsl::czstring<> device, TypedParams params, ModificationImpactFlag flags) noexcept;

    bool setLifecycleAction(Lifecycle type, LifecycleAction action, ModificationImpactFlag flags) noexcept;

    bool setMemoryFlags(unsigned long memory, MemoryModFlag flags) noexcept;

    bool setMemoryParameters(TypedParams params, ModificationImpactFlag flags) noexcept;

    bool setNumaParameters(TypedParams params, ModificationImpactFlag flags) noexcept;

    bool setPerfEvents(TypedParams params, ModificationImpactFlag flags) noexcept;

    bool setSchedulerParameters(TypedParams params) noexcept;

    bool setSchedulerParameters(TypedParams params, ModificationImpactFlag flags) noexcept;

    bool setMetadata(MetadataType type, gsl::czstring<> metadata, gsl::czstring<> key, gsl::czstring<> uri, ModificationImpactFlag flags) noexcept;

    bool setTime(long long seconds, unsigned int nseconds, SetTimeFlag flags) noexcept;

    bool setUserPassword(gsl::czstring<> user, gsl::czstring<> password, SetUserPasswordFlag flags) noexcept;

    bool setVcpu(gsl::czstring<> vcpumap, bool state, ModificationImpactFlag flags) noexcept;

    bool setVcpus(unsigned int nvcpus) noexcept;

    bool setVcpus(unsigned int nvcpus, VCpuFlag flags) noexcept;

    bool shutdown() noexcept;

    bool shutdown(ShutdownFlag flag) noexcept;

    bool suspend() noexcept;

    bool undefine() noexcept;

    bool undefine(UndefineFlag) noexcept;

    bool updateDeviceFlags(gsl::czstring<> xml, DeviceModifyFlag flags) noexcept;

    [[nodiscard]] static Domain createXML(Connection&, gsl::czstring<> xml, CreateFlag flags);
    [[nodiscard]] static Domain createXML(Connection&, gsl::czstring<> xml);

    // [[nodiscard]] static Domain defineXML();
};
}
#include "virt_wrap/enums/Domain/Domain.hpp"
namespace virt {
class Domain::Stats::Record {
    friend Connection;

    Domain dom;
    std::vector<TypedParameter> params{};

    explicit Record(const virDomainStatsRecord&) noexcept;

  public:
};

class Domain::StateWReason : public std::variant<StateReason::NoState, StateReason::Running, StateReason::Blocked, StateReason::Paused,
                                                 StateReason::Shutdown, StateReason::Shutoff, StateReason::Crashed, StateReason::PMSuspended> {
    constexpr State state() const noexcept { return State(EHTag{}, this->index()); }
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
    friend Domain::Interface;
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
    [[nodiscard]] constexpr JobType type() const noexcept { return JobType{virDomainJobInfo::type}; }
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