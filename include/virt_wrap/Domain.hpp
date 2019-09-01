//
// Created by _as on 2019-01-31.
//

#pragma once

#include <filesystem>
#include <stdexcept>
#include <variant>
#include <vector>
#include <gsl/gsl>
#include "../cexpr_algs.hpp"
#include <libvirt/libvirt-domain.h>
#include "CpuMap.hpp"
#include "GFlags.hpp"
#include "decls.hpp"
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

int virDomainOpenChannel(virDomainPtr dom, const char* name, virStreamPtr st, unsigned int flags);
int virDomainOpenConsole(virDomainPtr dom, const char* dev_name, virStreamPtr st, unsigned int flags);

char* virDomainScreenshot(virDomainPtr domain, virStreamPtr stream, unsigned int screen, unsigned int flags);

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
            // IOTHREAD	=	VIR_DOMAIN_STATS_IOTHREAD, // iothread poll info
        };

        class Record;
    };
    enum class BlockCommitFlag {
        SHALLOW = VIR_DOMAIN_BLOCK_COMMIT_SHALLOW,   /* nullptr base means next backing file, not whole chain */
        DELETE = VIR_DOMAIN_BLOCK_COMMIT_DELETE,     /* Delete any files that are now invalid after their contents have been committed */
        ACTIVE = VIR_DOMAIN_BLOCK_COMMIT_ACTIVE,     /* Allow a two-phase commit when top is the active layer */
        RELATIVE = VIR_DOMAIN_BLOCK_COMMIT_RELATIVE, /* keep the backing chain referenced using relative names */
        BANDWIDTH_BYTES = VIR_DOMAIN_BLOCK_COMMIT_BANDWIDTH_BYTES, /* bandwidth in bytes/s instead of MiB/s */
    };
    class BlockCommitFlagsC : public EnumSetHelper<BlockCommitFlagsC, BlockCommitFlag> {
        using Base = EnumSetHelper<BlockCommitFlagsC, BlockCommitFlag>;
        friend Base;
        constexpr static std::array values = {"shallow", "delete", "active", "relative", "bandwitdh_bytes"};
    } constexpr static BlockCommitFlags;
    enum class BlockCopyFlag {
        SHALLOW = VIR_DOMAIN_BLOCK_COPY_SHALLOW,             /* Limit copy to top of source backing chain */
        REUSE_EXT = VIR_DOMAIN_BLOCK_COPY_REUSE_EXT,         /* Reuse existing external file for a copy */
        TRANSIENT_JOB = VIR_DOMAIN_BLOCK_COPY_TRANSIENT_JOB, /* Don't force usage of recoverable job for the copy operation */
    };
    class BlockCopyFlagsC : public EnumSetHelper<BlockCopyFlagsC, BlockCopyFlag> {
        using Base = EnumSetHelper<BlockCopyFlagsC, BlockCopyFlag>;
        friend Base;
        constexpr static std::array values = {"shallow", "reuse_ext", "transient_job"};
    } constexpr static BlockCopyFlags;
    enum class BlockJobAbortFlag {
        ASYNC = VIR_DOMAIN_BLOCK_JOB_ABORT_ASYNC,
        PIVOT = VIR_DOMAIN_BLOCK_JOB_ABORT_PIVOT,
    };
    class BlockJobAbortFlagsC : public EnumSetHelper<BlockJobAbortFlagsC, BlockJobAbortFlag> {
        using Base = EnumSetHelper<BlockJobAbortFlagsC, BlockJobAbortFlag>;
        friend Base;
        constexpr static std::array values = {"async", "pivot"};
    } constexpr static BlockJobAbortFlags{};
    enum class BlockJobInfoFlag {
        BANDWIDTH_BYTES = VIR_DOMAIN_BLOCK_JOB_INFO_BANDWIDTH_BYTES, /* bandwidth in bytes/s */
    };
    class BlockJobInfoFlagsC : public EnumSetHelper<BlockJobInfoFlagsC, BlockJobInfoFlag> {
        using Base = EnumSetHelper<BlockJobInfoFlagsC, BlockJobInfoFlag>;
        friend Base;
        constexpr static std::array values = {"bandwidth_bytes"};
    } constexpr static BlockJobInfoFlags{};
    enum class BlockJobSetSpeedFlag {
        BLOCK_JOB_SPEED_BANDWIDTH_BYTES = VIR_DOMAIN_BLOCK_JOB_SPEED_BANDWIDTH_BYTES, /* bandwidth in bytes/s instead of MiB/s */
    };
    class BlockJobSetSpeedFlagsC : public EnumSetHelper<BlockJobSetSpeedFlagsC, BlockJobSetSpeedFlag> {
        using Base = EnumSetHelper<BlockJobSetSpeedFlagsC, BlockJobSetSpeedFlag>;
        friend Base;
        constexpr static std::array values = {"bandwidth_bytes"};
    } constexpr static BlockJobSetSpeedFlags{};
    enum class BlockPullFlag {
        BLOCK_PULL_BANDWIDTH_BYTES = VIR_DOMAIN_BLOCK_PULL_BANDWIDTH_BYTES, /* bandwidth in bytes/s instead of MiB/s */
    };
    class BlockPullFlagsC : public EnumSetHelper<BlockPullFlagsC, BlockPullFlag> {
        using Base = EnumSetHelper<BlockPullFlagsC, BlockPullFlag>;
        friend Base;
        constexpr static std::array values = {"", "", "", "", "", "", "bandwidth_bytes"};
    } constexpr static BlockPullFlags{};
    enum class BlockRebaseFlag {
        SHALLOW = VIR_DOMAIN_BLOCK_REBASE_SHALLOW,                 /* Limit copy to top of source backing chain */
        REUSE_EXT = VIR_DOMAIN_BLOCK_REBASE_REUSE_EXT,             /* Reuse existing external file for a copy */
        COPY_RAW = VIR_DOMAIN_BLOCK_REBASE_COPY_RAW,               /* Make destination file raw */
        COPY = VIR_DOMAIN_BLOCK_REBASE_COPY,                       /* Start a copy job */
        RELATIVE = VIR_DOMAIN_BLOCK_REBASE_RELATIVE,               /* Keep backing chain referenced using relative names */
        COPY_DEV = VIR_DOMAIN_BLOCK_REBASE_COPY_DEV,               /* Treat destination as block device instead of file */
        BANDWIDTH_BYTES = VIR_DOMAIN_BLOCK_REBASE_BANDWIDTH_BYTES, /* bandwidth in bytes/s instead of MiB/s */
    };
    class BlockRebaseFlagsC : public EnumSetHelper<BlockRebaseFlagsC, BlockRebaseFlag> {
        using Base = EnumSetHelper<BlockRebaseFlagsC, BlockRebaseFlag>;
        friend Base;
        constexpr static std::array values = {"shallow", "reuse_ext", "copy_raw", "copy", "relative", "copy_dev", "bandwidth_bytes"};
    } constexpr static BlockRebaseFlags{};
    enum class BlockResizeFlag {
        OCK_RESIZE_BYTES = VIR_DOMAIN_BLOCK_RESIZE_BYTES, /* size in bytes instead of KiB */
    };
    class BlockResizeFlagsC : public EnumSetHelper<BlockResizeFlagsC, BlockResizeFlag> {
        using Base = EnumSetHelper<BlockResizeFlagsC, BlockResizeFlag>;
        friend Base;
        constexpr static std::array values = {"bytes"};
    } constexpr static BlockResizeFlags{};

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
    enum class CreateFlag {
        NONE = VIR_DOMAIN_NONE,
        PAUSED = VIR_DOMAIN_START_PAUSED,
        AUTODESTROY = VIR_DOMAIN_START_AUTODESTROY,
        BYPASS_CACHE = VIR_DOMAIN_START_BYPASS_CACHE,
        FORCE_BOOT = VIR_DOMAIN_START_FORCE_BOOT,
        VALIDATE = VIR_DOMAIN_START_VALIDATE
    };
    class CreateFlagsC : public EnumSetHelper<CreateFlagsC, CreateFlag> {
        using Base = EnumSetHelper<CreateFlagsC, CreateFlag>;
        friend Base;
        constexpr static std::array values = {"paused", "autodestroy", "bypass_cache", "force_boot", "validate"};
    };
    constexpr static CreateFlagsC CreateFlags{};
    enum class DestroyFlag {
        DEFAULT = VIR_DOMAIN_DESTROY_DEFAULT,   /* Default behavior - could lead to data loss!! */
        GRACEFUL = VIR_DOMAIN_DESTROY_GRACEFUL, /* only SIGTERM, no SIGKILL */
    };
    class DestroyFlagsC : public EnumSetHelper<DestroyFlagsC, DestroyFlag> {
        using Base = EnumSetHelper<DestroyFlagsC, DestroyFlag>;
        friend Base;
        constexpr static std::array values = {"graceful"};
    };
    constexpr static DestroyFlagsC DestroyFlags{};
    enum class DeviceModifyFlag {
        /* See ModificationImpactFlag for these flags.  */
        CURRENT = VIR_DOMAIN_DEVICE_MODIFY_CURRENT,
        LIVE = VIR_DOMAIN_DEVICE_MODIFY_LIVE,
        CONFIG = VIR_DOMAIN_DEVICE_MODIFY_CONFIG,

        /* Additionally, these flags may be bitwise-OR'd in.  */
        FORCE = VIR_DOMAIN_DEVICE_MODIFY_FORCE, /* Forcibly modify device (ex. force eject a cdrom) */
    };
    class DeviceModifyFlagsC : public EnumSetHelper<DeviceModifyFlagsC, DeviceModifyFlag> {
        using Base = EnumSetHelper<DeviceModifyFlagsC, DeviceModifyFlag>;
        friend Base;
        constexpr static std::array values = {"live", "config", "force"};
    } constexpr static DeviceModifyFlags{};
    enum class GetJobStatsFlag {
        COMPLETED = VIR_DOMAIN_JOB_STATS_COMPLETED, /* return stats of a recently completed job */
    };
    class GetJobStatsFlagsC : public EnumSetHelper<GetJobStatsFlagsC, GetJobStatsFlag> {
        using Base = EnumSetHelper<GetJobStatsFlagsC, GetJobStatsFlag>;
        friend Base;
        constexpr static std::array values = {"completed"};
    } constexpr static GetJobStatsFlags{};
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
    enum class KeycodeSet {
        LINUX = VIR_KEYCODE_SET_LINUX,
        XT = VIR_KEYCODE_SET_XT,
        ATSET1 = VIR_KEYCODE_SET_ATSET1,
        ATSET2 = VIR_KEYCODE_SET_ATSET2,
        ATSET3 = VIR_KEYCODE_SET_ATSET3,
        OSX = VIR_KEYCODE_SET_OSX,
        KBD = VIR_KEYCODE_SET_XT_KBD,
        USB = VIR_KEYCODE_SET_USB,
        WIN32 = VIR_KEYCODE_SET_WIN32,
        QNUM = VIR_KEYCODE_SET_QNUM,
    };
    class KeycodeSetsC : public EnumHelper<KeycodeSetsC, KeycodeSet> {
        using Base = EnumHelper<KeycodeSetsC, KeycodeSet>;
        friend Base;
        constexpr static std::array values = {"linux", "xt", "atset1", "atset2", "atset3", "osx", ""};
    } constexpr static KeycodeSets{};
    enum class Lifecycle : unsigned {
        POWEROFF = VIR_DOMAIN_LIFECYCLE_POWEROFF,
        REBOOT = VIR_DOMAIN_LIFECYCLE_REBOOT,
        CRASH = VIR_DOMAIN_LIFECYCLE_CRASH,
    };
    class LifecyclesC : public EnumHelper<LifecyclesC, Lifecycle> {
        using Base = EnumHelper<LifecyclesC, Lifecycle>;
        friend Base;
        constexpr static std::array values = {"poweroff", "reboot", "crash"};
    } constexpr static Lifecycles{};
    enum class LifecycleAction : unsigned {
        RESTART = VIR_DOMAIN_LIFECYCLE_ACTION_RESTART,
        RESTART_RENAME = VIR_DOMAIN_LIFECYCLE_ACTION_RESTART_RENAME,
        PRESERVE = VIR_DOMAIN_LIFECYCLE_ACTION_PRESERVE,
        COREDUMP_DESTROY = VIR_DOMAIN_LIFECYCLE_ACTION_COREDUMP_DESTROY,
        COREDUMP_RESTART = VIR_DOMAIN_LIFECYCLE_ACTION_COREDUMP_RESTART,
    };
    class LifecycleActionsC : public EnumHelper<LifecycleActionsC, LifecycleAction> {
        using Base = EnumHelper<LifecycleActionsC, LifecycleAction>;
        friend Base;
        constexpr static std::array values = {"destroy", "restart", "restart_rename", "preserve", "coredump_destroy", "coredump_restart"};
    } constexpr static LifecycleActions{};

    enum class MemoryModFlag {
        /* See virDomainModificationImpact for these flags.  */
        LIVE = VIR_DOMAIN_MEM_LIVE,
        CONFIG = VIR_DOMAIN_MEM_CONFIG,

        /* Additionally, these flags may be bitwise-OR'd in.  */
        MAXIMUM = VIR_DOMAIN_MEM_MAXIMUM, /* affect Max rather than current */
    };
    class MemoryModFlagsC : public EnumSetHelper<MemoryModFlagsC, MemoryModFlag> {
        using Base = EnumSetHelper<MemoryModFlagsC, MemoryModFlag>;
        friend Base;
        constexpr static std::array values = {"live", "config", "maximum"};
    } constexpr static MemoryModFlags{};
    enum class OpenGraphicsFlag {
        SKIPAUTH = VIR_DOMAIN_OPEN_GRAPHICS_SKIPAUTH,
    };
    class OpenGraphicsFlagsC : public EnumSetHelper<OpenGraphicsFlagsC, OpenGraphicsFlag> {
        using Base = EnumSetHelper<OpenGraphicsFlagsC, OpenGraphicsFlag>;
        friend Base;
        constexpr static std::array values = {"skipauth"};
    } constexpr static OpenGraphicsFlags{};
    enum class SaveImageXMLFlag : unsigned {
        SECURE = VIR_DOMAIN_XML_SECURE, /* dump security sensitive information too */
    };
    class SaveRestoreFlag;
    enum class ShutdownFlag {
        DEFAULT = VIR_DOMAIN_REBOOT_DEFAULT,               /* hypervisor choice */
        ACPI_POWER_BTN = VIR_DOMAIN_REBOOT_ACPI_POWER_BTN, /* Send ACPI event */
        GUEST_AGENT = VIR_DOMAIN_REBOOT_GUEST_AGENT,       /* Use guest agent */
        INITCTL = VIR_DOMAIN_REBOOT_INITCTL,               /* Use initctl */
        SIGNAL = VIR_DOMAIN_REBOOT_SIGNAL,                 /* Send a signal */
        PARAVIRT = VIR_DOMAIN_REBOOT_PARAVIRT,             /* Use paravirt guest control */
    };
    class ShutdownFlagsC : public EnumSetHelper<ShutdownFlagsC, ShutdownFlag> {
        using Base = EnumSetHelper<ShutdownFlagsC, ShutdownFlag>;
        friend Base;
        constexpr static std::array values = {"acpi_power_btn", "guest_agent", "initctl", "signal", "paravirt"};
    } constexpr static ShutdownFlags{};
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
    enum class MetadataType {
        DESCRIPTION = VIR_DOMAIN_METADATA_DESCRIPTION, /* Operate on <description> */
        TITLE = VIR_DOMAIN_METADATA_TITLE,             /* Operate on <title> */
        ELEMENT = VIR_DOMAIN_METADATA_ELEMENT,         /* Operate on <metadata> */
    };
    class MetadataTypesC : public EnumHelper<MetadataTypesC, MetadataType> {
        using Base = EnumHelper<MetadataTypesC, MetadataType>;
        friend Base;
        constexpr static std::array values = {"description", "title", "element"};
    } constexpr static MetadataTypes{};
    enum class MigrateFlag {
        LIVE = VIR_MIGRATE_LIVE,
        PEER2PEER = VIR_MIGRATE_PEER2PEER,
        TUNNELLED = VIR_MIGRATE_TUNNELLED,
        PERSIST_DEST = VIR_MIGRATE_PERSIST_DEST,
        UNDEFINE_SOURCE = VIR_MIGRATE_UNDEFINE_SOURCE,
        PAUSED = VIR_MIGRATE_PAUSED,
        NON_SHARED_DISK = VIR_MIGRATE_NON_SHARED_DISK,
        NON_SHARED_INC = VIR_MIGRATE_NON_SHARED_INC,
        CHANGE_PROTECTION = VIR_MIGRATE_CHANGE_PROTECTION,
        UNSAFE = VIR_MIGRATE_UNSAFE,
        OFFLINE = VIR_MIGRATE_OFFLINE,
        COMPRESSED = VIR_MIGRATE_COMPRESSED,
        ABORT_ON_ERROR = VIR_MIGRATE_ABORT_ON_ERROR,
        AUTO_CONVERGE = VIR_MIGRATE_AUTO_CONVERGE,
        RDMA_PIN_ALL = VIR_MIGRATE_RDMA_PIN_ALL,
        POSTCOPY = VIR_MIGRATE_POSTCOPY,
        TLS = VIR_MIGRATE_TLS,
    };
    class MigrateFlagsC : public EnumSetHelper<MigrateFlagsC, MigrateFlag> {
        using Base = EnumSetHelper<MigrateFlagsC, MigrateFlag>;
        friend Base;
        constexpr static std::array values = {
            "live",           "peer2peer",         "tunnelled", "persist_dest", "undefine_source", "paused",         "non_shared_disk",
            "non_shared_inc", "change_protection", "unsafe",    "offline",      "compressed",      "abort_on_error", "auto_converge",
            "rdma_pin_all",   "postcopy",          "tls"};
    } constexpr static MigrateFlags{};
    enum class ModificationImpactFlag {
        CURRENT = VIR_DOMAIN_AFFECT_CURRENT, /* Affect current domain state.  */
        LIVE = VIR_DOMAIN_AFFECT_LIVE,       /* Affect running domain state.  */
        CONFIG = VIR_DOMAIN_AFFECT_CONFIG,   /* Affect persistent domain state.  */
    };
    class ModificationImpactFlagsC : public EnumSetHelper<ModificationImpactFlagsC, ModificationImpactFlag> {
        using Base = EnumSetHelper<ModificationImpactFlagsC, ModificationImpactFlag>;
        friend Base;
        constexpr static std::array values = {"live", "config"};
    } constexpr static ModificationImpactFlags{};
    enum class ProcessSignal {
        NOP = VIR_DOMAIN_PROCESS_SIGNAL_NOP,   /* No constant in POSIX/Linux */
        HUP = VIR_DOMAIN_PROCESS_SIGNAL_HUP,   /* SIGHUP */
        INT = VIR_DOMAIN_PROCESS_SIGNAL_INT,   /* SIGINT */
        QUIT = VIR_DOMAIN_PROCESS_SIGNAL_QUIT, /* SIGQUIT */
        ILL = VIR_DOMAIN_PROCESS_SIGNAL_ILL,   /* SIGILL */
        TRAP = VIR_DOMAIN_PROCESS_SIGNAL_TRAP, /* SIGTRAP */
        ABRT = VIR_DOMAIN_PROCESS_SIGNAL_ABRT, /* SIGABRT */
        BUS = VIR_DOMAIN_PROCESS_SIGNAL_BUS,   /* SIGBUS */
        FPE = VIR_DOMAIN_PROCESS_SIGNAL_FPE,   /* SIGFPE */
        KILL = VIR_DOMAIN_PROCESS_SIGNAL_KILL, /* SIGKILL */

        USR1 = VIR_DOMAIN_PROCESS_SIGNAL_USR1,     /* SIGUSR1 */
        SEGV = VIR_DOMAIN_PROCESS_SIGNAL_SEGV,     /* SIGSEGV */
        USR2 = VIR_DOMAIN_PROCESS_SIGNAL_USR2,     /* SIGUSR2 */
        PIPE = VIR_DOMAIN_PROCESS_SIGNAL_PIPE,     /* SIGPIPE */
        ALRM = VIR_DOMAIN_PROCESS_SIGNAL_ALRM,     /* SIGALRM */
        TERM = VIR_DOMAIN_PROCESS_SIGNAL_TERM,     /* SIGTERM */
        STKFLT = VIR_DOMAIN_PROCESS_SIGNAL_STKFLT, /* Not in POSIX (SIGSTKFLT on Linux )*/
        CHLD = VIR_DOMAIN_PROCESS_SIGNAL_CHLD,     /* SIGCHLD */
        CONT = VIR_DOMAIN_PROCESS_SIGNAL_CONT,     /* SIGCONT */
        STOP = VIR_DOMAIN_PROCESS_SIGNAL_STOP,     /* SIGSTOP */

        TSTP = VIR_DOMAIN_PROCESS_SIGNAL_TSTP,     /* SIGTSTP */
        TTIN = VIR_DOMAIN_PROCESS_SIGNAL_TTIN,     /* SIGTTIN */
        TTOU = VIR_DOMAIN_PROCESS_SIGNAL_TTOU,     /* SIGTTOU */
        URG = VIR_DOMAIN_PROCESS_SIGNAL_URG,       /* SIGURG */
        XCPU = VIR_DOMAIN_PROCESS_SIGNAL_XCPU,     /* SIGXCPU */
        XFSZ = VIR_DOMAIN_PROCESS_SIGNAL_XFSZ,     /* SIGXFSZ */
        VTALRM = VIR_DOMAIN_PROCESS_SIGNAL_VTALRM, /* SIGVTALRM */
        PROF = VIR_DOMAIN_PROCESS_SIGNAL_PROF,     /* SIGPROF */
        WINCH = VIR_DOMAIN_PROCESS_SIGNAL_WINCH,   /* Not in POSIX (SIGWINCH on Linux) */
        POLL = VIR_DOMAIN_PROCESS_SIGNAL_POLL,     /* SIGPOLL (also known as SIGIO on Linux) */

        PWR = VIR_DOMAIN_PROCESS_SIGNAL_PWR, /* Not in POSIX (SIGPWR on Linux) */
        SYS = VIR_DOMAIN_PROCESS_SIGNAL_SYS, /* SIGSYS (also known as SIGUNUSED on Linux) */
        RT0 = VIR_DOMAIN_PROCESS_SIGNAL_RT0, /* SIGRTMIN */
        RT1 = VIR_DOMAIN_PROCESS_SIGNAL_RT1, /* SIGRTMIN + 1 */
        RT2 = VIR_DOMAIN_PROCESS_SIGNAL_RT2, /* SIGRTMIN + 2 */
        RT3 = VIR_DOMAIN_PROCESS_SIGNAL_RT3, /* SIGRTMIN + 3 */
        RT4 = VIR_DOMAIN_PROCESS_SIGNAL_RT4, /* SIGRTMIN + 4 */
        RT5 = VIR_DOMAIN_PROCESS_SIGNAL_RT5, /* SIGRTMIN + 5 */
        RT6 = VIR_DOMAIN_PROCESS_SIGNAL_RT6, /* SIGRTMIN + 6 */
        RT7 = VIR_DOMAIN_PROCESS_SIGNAL_RT7, /* SIGRTMIN + 7 */

        RT8 = VIR_DOMAIN_PROCESS_SIGNAL_RT8,   /* SIGRTMIN + 8 */
        RT9 = VIR_DOMAIN_PROCESS_SIGNAL_RT9,   /* SIGRTMIN + 9 */
        RT10 = VIR_DOMAIN_PROCESS_SIGNAL_RT10, /* SIGRTMIN + 10 */
        RT11 = VIR_DOMAIN_PROCESS_SIGNAL_RT11, /* SIGRTMIN + 11 */
        RT12 = VIR_DOMAIN_PROCESS_SIGNAL_RT12, /* SIGRTMIN + 12 */
        RT13 = VIR_DOMAIN_PROCESS_SIGNAL_RT13, /* SIGRTMIN + 13 */
        RT14 = VIR_DOMAIN_PROCESS_SIGNAL_RT14, /* SIGRTMIN + 14 */
        RT15 = VIR_DOMAIN_PROCESS_SIGNAL_RT15, /* SIGRTMIN + 15 */
        RT16 = VIR_DOMAIN_PROCESS_SIGNAL_RT16, /* SIGRTMIN + 16 */
        RT17 = VIR_DOMAIN_PROCESS_SIGNAL_RT17, /* SIGRTMIN + 17 */

        RT18 = VIR_DOMAIN_PROCESS_SIGNAL_RT18, /* SIGRTMIN + 18 */
        RT19 = VIR_DOMAIN_PROCESS_SIGNAL_RT19, /* SIGRTMIN + 19 */
        RT20 = VIR_DOMAIN_PROCESS_SIGNAL_RT20, /* SIGRTMIN + 20 */
        RT21 = VIR_DOMAIN_PROCESS_SIGNAL_RT21, /* SIGRTMIN + 21 */
        RT22 = VIR_DOMAIN_PROCESS_SIGNAL_RT22, /* SIGRTMIN + 22 */
        RT23 = VIR_DOMAIN_PROCESS_SIGNAL_RT23, /* SIGRTMIN + 23 */
        RT24 = VIR_DOMAIN_PROCESS_SIGNAL_RT24, /* SIGRTMIN + 24 */
        RT25 = VIR_DOMAIN_PROCESS_SIGNAL_RT25, /* SIGRTMIN + 25 */
        RT26 = VIR_DOMAIN_PROCESS_SIGNAL_RT26, /* SIGRTMIN + 26 */
        RT27 = VIR_DOMAIN_PROCESS_SIGNAL_RT27, /* SIGRTMIN + 27 */

        RT28 = VIR_DOMAIN_PROCESS_SIGNAL_RT28, /* SIGRTMIN + 28 */
        RT29 = VIR_DOMAIN_PROCESS_SIGNAL_RT29, /* SIGRTMIN + 29 */
        RT30 = VIR_DOMAIN_PROCESS_SIGNAL_RT30, /* SIGRTMIN + 30 */
        RT31 = VIR_DOMAIN_PROCESS_SIGNAL_RT31, /* SIGRTMIN + 31 */
        RT32 = VIR_DOMAIN_PROCESS_SIGNAL_RT32, /* SIGRTMIN + 32 / SIGRTMAX */
    };
    class ProcessSignalsC : public EnumHelper<ProcessSignalsC, ProcessSignal> {
        using Base = EnumHelper<ProcessSignalsC, ProcessSignal>;
        friend Base;
        constexpr static std::array values = {"nop",  "hup",  "int",  "quit", "ill",    "trap",   "abrt",  "bus",  "fpe",  "kill", "usr1",
                                              "segv", "usr2", "pipe", "alrm", "term",   "stkflt", "chld",  "cont", "stop", "tstp", "ttin",
                                              "ttou", "urg",  "xcpu", "xfsz", "vtalrm", "prof",   "winch", "poll", "pwr",  "sys",  "rt0",
                                              "rt1",  "rt2",  "rt3",  "rt4",  "rt5",    "rt6",    "rt7",   "rt8",  "rt9",  "rt10", "rt11",
                                              "rt12", "rt13", "rt14", "rt15", "rt16",   "rt17",   "rt18",  "rt19", "rt20", "rt21", "rt22",
                                              "rt23", "rt24", "rt25", "rt26", "rt27",   "rt28",   "rt29",  "rt30", "rt31", "rt32"};
    } constexpr static ProcessSignals{};
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
    enum class XmlFlag {
        DEFAULT = 0,
        SECURE = VIR_DOMAIN_XML_SECURE,         /* dump security sensitive information too */
        INACTIVE = VIR_DOMAIN_XML_INACTIVE,     /* dump inactive domain information */
        UPDATE_CPU = VIR_DOMAIN_XML_UPDATE_CPU, /* update guest CPU requirements according to host CPU */
        MIGRATABLE = VIR_DOMAIN_XML_MIGRATABLE, /* dump XML suitable for migration */
    };
    class XmlFlagsC : public EnumSetHelper<XmlFlagsC, XmlFlag> {
        using Base = EnumSetHelper<XmlFlagsC, XmlFlag>;
        friend Base;
        constexpr static std::array values = {"secure", "inactive", "update_cpu", "migratable"};
    } constexpr static XmlFlags{};
    enum class State : int {
        NOSTATE = VIR_DOMAIN_NOSTATE,         /* no state */
        RUNNING = VIR_DOMAIN_RUNNING,         /* the domain is running */
        BLOCKED = VIR_DOMAIN_BLOCKED,         /* the domain is blocked on resource */
        PAUSED = VIR_DOMAIN_PAUSED,           /* the domain is paused by user */
        SHUTDOWN = VIR_DOMAIN_SHUTDOWN,       /* the domain is being shut down */
        SHUTOFF = VIR_DOMAIN_SHUTOFF,         /* the domain is shut off */
        CRASHED = VIR_DOMAIN_CRASHED,         /* the domain is crashed */
        PMSUSPENDED = VIR_DOMAIN_PMSUSPENDED, /* the domain is suspended by guest power management */
        ENUM_END
    };
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
    class StatesC : public EnumHelper<StatesC, State> {
        using Base = EnumHelper<StatesC, State>;
        friend Base;
        constexpr static std::array values = {"No State",      "Running", "Blocked", "Paused",
                                              "Shutting down", "Shutoff", "Crashed", "Power Management Suspended"};
    };
    constexpr static StatesC States{};
    class SaveRestoreFlagsC : public EnumSetHelper<SaveRestoreFlagsC, SaveRestoreFlag> {
        using Base = EnumSetHelper<SaveRestoreFlagsC, State>;
        friend Base;
        constexpr static std::array values = {"bypass_cache", "running", "paused"};
    };
    constexpr static SaveRestoreFlagsC SaveRestoreFlags{};
    enum class UndefineFlag {
        MANAGED_SAVE = VIR_DOMAIN_UNDEFINE_MANAGED_SAVE,             /* Also remove any managed save */
        SNAPSHOTS_METADATA = VIR_DOMAIN_UNDEFINE_SNAPSHOTS_METADATA, /* If last use of domain, then also remove any snapshot metadata */
        NVRAM = VIR_DOMAIN_UNDEFINE_NVRAM,                           /* Also remove any nvram file */
        KEEP_NVRAM = VIR_DOMAIN_UNDEFINE_KEEP_NVRAM,                 /* Keep nvram file */
    };
    class UndefineFlagsC : public EnumSetHelper<UndefineFlagsC, UndefineFlag> {
        using Base = EnumSetHelper<UndefineFlagsC, UndefineFlag>;
        friend Base;
        constexpr static std::array values = {"managed_save", "snapshots_metadata", "nvram", "keep_nvram"};
    };
    constexpr static UndefineFlagsC UndefineFlags;

    using BlockStats = virDomainBlockStatsStruct;

    using MITPFlags = EnumSetTie<ModificationImpactFlag, TypedParameterFlag>;
    using MITPFlagsC = EnumSetCTie<EnumSetTie<ModificationImpactFlag, TypedParameterFlag>, std::tuple<ModificationImpactFlagsC, TypedParameterFlagC>>;

    constexpr inline explicit Domain(virDomainPtr ptr = nullptr) noexcept;

    Domain(const Domain&) = delete;

    constexpr inline Domain(Domain&&) noexcept;

    Domain& operator=(const Domain&) = delete;

    inline Domain& operator=(Domain&&) noexcept;

    inline ~Domain() noexcept;

    constexpr inline explicit operator bool() const noexcept;

    bool abortJob() noexcept;

    bool addIOThread(unsigned int iothread_id, ModificationImpactFlag flags = ModificationImpactFlag::CURRENT) noexcept;

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

    bool detachDevice(gsl::czstring<> xml, DeviceModifyFlag flag = DeviceModifyFlag::LIVE) noexcept;

    bool detachDeviceAlias(gsl::czstring<> alias, DeviceModifyFlag flag = DeviceModifyFlag::LIVE) noexcept;

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

    // [[nodiscard]] int getEmulatorPinInfo(unsigned char* cpumap, int maplen, unsigned int flags); // TODO figure out the intended way of using
    // this

    [[nodiscard]] auto getFSInfo() const noexcept;

    [[nodiscard]] std::vector<FSInfo> extractFSInfo() const;

    [[nodiscard]] auto getJobStats(GetJobStatsFlag flags = GetJobStatsFlag{0}) const noexcept;

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

    [[nodiscard]] UniqueZstring getMetadata(MetadataType type, gsl::czstring<> ns,
                                            ModificationImpactFlag flags = ModificationImpactFlag::CURRENT) const noexcept;

    [[nodiscard]] std::string extractMetadata(MetadataType type, gsl::czstring<> ns,
                                              ModificationImpactFlag flags = ModificationImpactFlag::CURRENT) const;

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

    [[nodiscard]] gsl::czstring<> getXMLDesc(XmlFlag flag = XmlFlag::DEFAULT) const noexcept;

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

    bool openGraphics(unsigned int idx, int fd, OpenGraphicsFlag flags) const noexcept;

    [[nodiscard]] int openGraphicsFD(unsigned int idx, OpenGraphicsFlag flags) const noexcept;

    bool pinEmulator(CpuMap cpumap, ModificationImpactFlag flags) noexcept;

    bool pinIOThread(unsigned int iothread_id, CpuMap cpumap, ModificationImpactFlag flags) noexcept;

    bool pinVcpu(unsigned int vcpu, CpuMap cpumap) noexcept;

    bool pinVcpuFlags(unsigned int vcpu, CpuMap cpumap, ModificationImpactFlag flags) noexcept;

    bool sendKey(KeycodeSet codeset, unsigned int holdtime, gsl::span<unsigned int> keycodes) noexcept;

    bool sendProcessSignal(long long pid_value, ProcessSignal signum) noexcept;

    bool setMaxMemory(unsigned long);

    bool setMemory(unsigned long);

    bool setMemoryStatsPeriod(int period, MemoryModFlag flags) noexcept;

    bool reboot(ShutdownFlag flags = ShutdownFlag::DEFAULT);

    bool reset();

    bool rename(gsl::czstring<>);

    bool resume() noexcept;

    bool save(gsl::czstring<> to) noexcept;

    bool save(gsl::czstring<> to, gsl::czstring<> dxml, SaveRestoreFlag flags) noexcept;

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

    [[nodiscard]] static Domain createXML(Connection&, gsl::czstring<> xml, CreateFlag flags = CreateFlag::NONE);

    // [[nodiscard]] static Domain defineXML();
};

// HELP ME I NEED GENERATING METACLASSES
// otherwise I'll just use macros...
class Domain::SaveRestoreFlag {
    struct EBase {};
    struct DEFAULT_t : EBase {
        constexpr static auto value = 0;
    };
    struct BYPASS_CACHE_t {
        constexpr static auto value = VIR_DOMAIN_SAVE_BYPASS_CACHE;
    };
    struct RUNNING_t {
        constexpr static auto value = VIR_DOMAIN_SAVE_RUNNING;
    };
    struct PAUSED_t {
        constexpr static auto value = VIR_DOMAIN_SAVE_PAUSED;
    };

    unsigned underlying{};

    friend constexpr SaveRestoreFlag operator|(DEFAULT_t, BYPASS_CACHE_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(DEFAULT_t, RUNNING_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(DEFAULT_t, PAUSED_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(BYPASS_CACHE_t, DEFAULT_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(BYPASS_CACHE_t, RUNNING_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(BYPASS_CACHE_t, PAUSED_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(RUNNING_t, DEFAULT_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(RUNNING_t, BYPASS_CACHE_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(PAUSED_t, DEFAULT_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(PAUSED_t, BYPASS_CACHE_t) noexcept;

    friend constexpr unsigned impl_to_integral(SaveRestoreFlag) noexcept;

  public:
    constexpr static auto DEFAULT = DEFAULT_t{};
    constexpr static auto BYPASS_CACHE = BYPASS_CACHE_t{};
    constexpr static auto RUNNING = RUNNING_t{};
    constexpr static auto PAUSED = PAUSED_t{};

    constexpr SaveRestoreFlag() = default;
    constexpr SaveRestoreFlag(unsigned val) noexcept : underlying(val) {}
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<EBase, T>>> constexpr SaveRestoreFlag(T) noexcept : underlying(T::value) {}
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<EBase, T>>> constexpr SaveRestoreFlag& operator=(T) noexcept {
        underlying = T::value;
        return *this;
    }

    constexpr SaveRestoreFlag& operator|=(DEFAULT_t) noexcept {
        underlying |= 0;
        return *this;
    }
    constexpr SaveRestoreFlag& operator|=(BYPASS_CACHE_t) noexcept {
        underlying |= VIR_DOMAIN_SAVE_BYPASS_CACHE;
        return *this;
    }
    constexpr SaveRestoreFlag& operator|=(RUNNING_t) {
        underlying &= ~VIR_DOMAIN_SAVE_PAUSED;
        underlying |= VIR_DOMAIN_SAVE_RUNNING;
        return *this;
    }
    constexpr SaveRestoreFlag& operator|=(PAUSED_t) {
        underlying &= ~VIR_DOMAIN_SAVE_RUNNING;
        underlying |= VIR_DOMAIN_SAVE_PAUSED;
        return *this;
    }

    constexpr SaveRestoreFlag operator|(SaveRestoreFlag f) const noexcept;
    constexpr SaveRestoreFlag operator|(DEFAULT_t) const noexcept { return *this; }
    constexpr SaveRestoreFlag operator|(BYPASS_CACHE_t) const noexcept { return {underlying | VIR_DOMAIN_SAVE_BYPASS_CACHE}; }
    constexpr SaveRestoreFlag operator|(RUNNING_t) const noexcept { return {(underlying & ~VIR_DOMAIN_SAVE_PAUSED) | VIR_DOMAIN_SAVE_RUNNING}; }
    constexpr SaveRestoreFlag operator|(PAUSED_t) const noexcept { return {(underlying & ~VIR_DOMAIN_SAVE_RUNNING) | VIR_DOMAIN_SAVE_PAUSED}; }
};

constexpr Domain::SaveRestoreFlag operator|(Domain::SaveRestoreFlag::DEFAULT_t, Domain::SaveRestoreFlag::BYPASS_CACHE_t) noexcept {
    return {VIR_DOMAIN_SAVE_BYPASS_CACHE};
}
constexpr Domain::SaveRestoreFlag operator|(Domain::SaveRestoreFlag::DEFAULT_t, Domain::SaveRestoreFlag::RUNNING_t) noexcept {
    return {VIR_DOMAIN_SAVE_RUNNING};
}
constexpr Domain::SaveRestoreFlag operator|(Domain::SaveRestoreFlag::DEFAULT_t, Domain::SaveRestoreFlag::PAUSED_t) noexcept {
    return {VIR_DOMAIN_SAVE_PAUSED};
}
constexpr Domain::SaveRestoreFlag operator|(Domain::SaveRestoreFlag::BYPASS_CACHE_t, Domain::SaveRestoreFlag::DEFAULT_t) noexcept {
    return {VIR_DOMAIN_SAVE_BYPASS_CACHE};
}
constexpr Domain::SaveRestoreFlag operator|(Domain::SaveRestoreFlag::BYPASS_CACHE_t, Domain::SaveRestoreFlag::RUNNING_t) noexcept {
    return {VIR_DOMAIN_SAVE_BYPASS_CACHE | VIR_DOMAIN_SAVE_RUNNING};
}
constexpr Domain::SaveRestoreFlag operator|(Domain::SaveRestoreFlag::BYPASS_CACHE_t, Domain::SaveRestoreFlag::PAUSED_t) noexcept {
    return {VIR_DOMAIN_SAVE_BYPASS_CACHE | VIR_DOMAIN_SAVE_PAUSED};
}
constexpr Domain::SaveRestoreFlag operator|(Domain::SaveRestoreFlag::RUNNING_t, Domain::SaveRestoreFlag::DEFAULT_t) noexcept {
    return {VIR_DOMAIN_SAVE_RUNNING};
}
constexpr Domain::SaveRestoreFlag operator|(Domain::SaveRestoreFlag::RUNNING_t, Domain::SaveRestoreFlag::BYPASS_CACHE_t) noexcept {
    return {VIR_DOMAIN_SAVE_BYPASS_CACHE | VIR_DOMAIN_SAVE_RUNNING};
}
constexpr Domain::SaveRestoreFlag operator|(Domain::SaveRestoreFlag::PAUSED_t, Domain::SaveRestoreFlag::DEFAULT_t) noexcept {
    return {VIR_DOMAIN_SAVE_PAUSED};
}
constexpr Domain::SaveRestoreFlag operator|(Domain::SaveRestoreFlag::PAUSED_t, Domain::SaveRestoreFlag::BYPASS_CACHE_t) noexcept {
    return {VIR_DOMAIN_SAVE_BYPASS_CACHE | VIR_DOMAIN_SAVE_PAUSED};
}

constexpr unsigned impl_to_integral(virt::Domain::SaveRestoreFlag f) noexcept { return f.underlying; }

class Domain::Stats::Record {
    friend Connection;

    Domain dom;
    std::vector<TypedParameter> params{};

    explicit Record(const virDomainStatsRecord&) noexcept;

  public:
};

class Domain::StateWReason : public std::variant<StateReason::NoState, StateReason::Running, StateReason::Blocked, StateReason::Paused,
                                                 StateReason::Shutdown, StateReason::Shutoff, StateReason::Crashed, StateReason::PMSuspended> {
    constexpr State state() const noexcept { return State(this->index()); }
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

[[nodiscard]] constexpr inline Domain::BlockCommitFlag operator|(Domain::BlockCommitFlag lhs, Domain::BlockCommitFlag rhs) noexcept;
constexpr inline Domain::BlockCommitFlag& operator|=(Domain::BlockCommitFlag& lhs, Domain::BlockCommitFlag rhs) noexcept;
[[nodiscard]] constexpr Domain::BlockJobAbortFlag operator|(Domain::BlockJobAbortFlag lhs, Domain::BlockJobAbortFlag rhs) noexcept;
constexpr Domain::BlockJobAbortFlag& operator|=(Domain::BlockJobAbortFlag& lhs, Domain::BlockJobAbortFlag rhs) noexcept;
[[nodiscard]] constexpr Domain::BlockJobInfoFlag operator|(Domain::BlockJobInfoFlag lhs, Domain::BlockJobInfoFlag rhs) noexcept;
constexpr Domain::BlockJobInfoFlag& operator|=(Domain::BlockJobInfoFlag& lhs, Domain::BlockJobInfoFlag rhs) noexcept;
[[nodiscard]] constexpr Domain::BlockJobSetSpeedFlag operator|(Domain::BlockJobSetSpeedFlag lhs, Domain::BlockJobSetSpeedFlag rhs) noexcept;
constexpr Domain::BlockJobSetSpeedFlag& operator|=(Domain::BlockJobSetSpeedFlag& lhs, Domain::BlockJobSetSpeedFlag rhs) noexcept;
[[nodiscard]] constexpr Domain::BlockPullFlag operator|(Domain::BlockPullFlag lhs, Domain::BlockPullFlag rhs) noexcept;
constexpr Domain::BlockPullFlag& operator|=(Domain::BlockPullFlag& lhs, Domain::BlockPullFlag rhs) noexcept;
[[nodiscard]] constexpr Domain::BlockRebaseFlag operator|(Domain::BlockRebaseFlag lhs, Domain::BlockRebaseFlag rhs) noexcept;
constexpr Domain::BlockRebaseFlag& operator|=(Domain::BlockRebaseFlag& lhs, Domain::BlockRebaseFlag rhs) noexcept;
[[nodiscard]] constexpr Domain::BlockResizeFlag operator|(Domain::BlockResizeFlag lhs, Domain::BlockResizeFlag rhs) noexcept;
constexpr Domain::BlockResizeFlag& operator|=(Domain::BlockResizeFlag& lhs, Domain::BlockResizeFlag rhs) noexcept;
[[nodiscard]] constexpr inline Domain::CoreDump::Flag operator|(Domain::CoreDump::Flag lhs, Domain::CoreDump::Flag rhs) noexcept;
[[nodiscard]] constexpr Domain::DeviceModifyFlag operator|(Domain::DeviceModifyFlag lhs, Domain::DeviceModifyFlag rhs) noexcept;
constexpr Domain::DeviceModifyFlag& operator|=(Domain::DeviceModifyFlag& lhs, Domain::DeviceModifyFlag rhs) noexcept;
[[nodiscard]] constexpr inline Domain::GetAllDomainStatsFlag operator|(Domain::GetAllDomainStatsFlag lhs, Domain::GetAllDomainStatsFlag rhs) noexcept;
constexpr inline Domain::GetAllDomainStatsFlag operator|=(Domain::GetAllDomainStatsFlag& lhs, Domain::GetAllDomainStatsFlag rhs) noexcept;
[[nodiscard]] constexpr Domain::GetJobStatsFlag operator|(Domain::GetJobStatsFlag lhs, Domain::GetJobStatsFlag rhs) noexcept;
constexpr Domain::GetJobStatsFlag& operator|=(Domain::GetJobStatsFlag& lhs, Domain::GetJobStatsFlag rhs) noexcept;
[[nodiscard]] constexpr inline Domain::ShutdownFlag operator|(Domain::ShutdownFlag lhs, Domain::ShutdownFlag rhs) noexcept;
constexpr inline Domain::ShutdownFlag& operator|=(Domain::ShutdownFlag& lhs, Domain::ShutdownFlag rhs) noexcept;
[[nodiscard]] constexpr inline Domain::StatsType operator|(Domain::StatsType lhs, Domain::StatsType rhs) noexcept;
constexpr inline Domain::StatsType operator|=(Domain::StatsType& lhs, Domain::StatsType rhs) noexcept;
[[nodiscard]] constexpr inline Domain::MemoryModFlag operator|(Domain::MemoryModFlag lhs, Domain::MemoryModFlag rhs) noexcept;
constexpr inline Domain::MemoryModFlag operator|=(Domain::MemoryModFlag& lhs, Domain::MemoryModFlag rhs) noexcept;
[[nodiscard]] constexpr inline Domain::ModificationImpactFlag operator|(Domain::ModificationImpactFlag lhs,
                                                                        Domain::ModificationImpactFlag rhs) noexcept;
[[nodiscard]] constexpr Domain::OpenGraphicsFlag operator|(Domain::OpenGraphicsFlag lhs, Domain::OpenGraphicsFlag rhs) noexcept;
constexpr Domain::OpenGraphicsFlag& operator|=(Domain::OpenGraphicsFlag& lhs, Domain::OpenGraphicsFlag rhs) noexcept;
[[nodiscard]] constexpr inline Domain::VCpuFlag operator|(Domain::VCpuFlag lhs, Domain::VCpuFlag rhs) noexcept;
constexpr inline Domain::VCpuFlag& operator|=(Domain::VCpuFlag& lhs, Domain::VCpuFlag rhs) noexcept;
[[nodiscard]] constexpr inline Domain::Stats::Types operator|(Domain::Stats::Types lhs, Domain::Stats::Types rhs) noexcept;

constexpr inline Domain::UndefineFlag operator|(virt::Domain::UndefineFlag lhs, virt::Domain::UndefineFlag rhs) noexcept;
constexpr inline Domain::UndefineFlag& operator|=(virt::Domain::UndefineFlag& lhs, virt::Domain::UndefineFlag rhs) noexcept;

constexpr unsigned to_integral(virt::Domain::SaveRestoreFlag f) noexcept { return virt::impl_to_integral(f); }
} // namespace virt
