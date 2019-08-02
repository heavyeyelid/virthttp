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

int virDomainBlockCommit(virDomainPtr dom, const char* disk, const char* base, const char* top, unsigned long bandwidth, unsigned int flags);
int virDomainBlockCopy(virDomainPtr dom, const char* disk, const char* destxml, virTypedParameterPtr params, int nparams, unsigned int flags);
int virDomainBlockJobAbort(virDomainPtr dom, const char* disk, unsigned int flags);
int virDomainBlockJobSetSpeed(virDomainPtr dom, const char* disk, unsigned long bandwidth, unsigned int flags);
int virDomainBlockPeek(virDomainPtr dom, const char* disk, unsigned long long offset, size_t size, void* buffer, unsigned int flags);
int virDomainBlockPull(virDomainPtr dom, const char* disk, unsigned long bandwidth, unsigned int flags);
int virDomainBlockRebase(virDomainPtr dom, const char* disk, const char* base, unsigned long bandwidth, unsigned int flags);
int virDomainBlockResize(virDomainPtr dom, const char* disk, unsigned long long size, unsigned int flags);
int virDomainBlockStats(virDomainPtr dom, const char* disk, virDomainBlockStatsPtr stats, size_t size);
int virDomainBlockStatsFlags(virDomainPtr dom, const char* disk, virTypedParameterPtr params, int* nparams, unsigned int flags);

int virDomainGetBlkioParameters(virDomainPtr domain, virTypedParameterPtr params, int* nparams, unsigned int flags);

int virDomainGetBlockInfo(virDomainPtr domain, const char* disk, virDomainBlockInfoPtr info, unsigned int flags);
int virDomainGetBlockIoTune(virDomainPtr dom, const char* disk, virTypedParameterPtr params, int* nparams, unsigned int flags);
int virDomainGetBlockJobInfo(virDomainPtr dom, const char* disk, virDomainBlockJobInfoPtr info, unsigned int flags);

int virDomainGetCPUStats(virDomainPtr domain, virTypedParameterPtr params, unsigned int nparams, int start_cpu, unsigned int ncpus,
                         unsigned int flags);

int virDomainGetGuestVcpus(virDomainPtr domain, virTypedParameterPtr* params, unsigned int* nparams, unsigned int flags);

int virDomainGetInterfaceParameters(virDomainPtr domain, const char* device, virTypedParameterPtr params, int* nparams, unsigned int flags);

int virDomainGetJobStats(virDomainPtr domain, int* type, virTypedParameterPtr* params, int* nparams, unsigned int flags);
int virDomainGetLaunchSecurityInfo(virDomainPtr domain, virTypedParameterPtr* params, int* nparams, unsigned int flags);

int virDomainGetMemoryParameters(virDomainPtr domain, virTypedParameterPtr params, int* nparams, unsigned int flags);

int virDomainGetNumaParameters(virDomainPtr domain, virTypedParameterPtr params, int* nparams, unsigned int flags);
int virDomainGetPerfEvents(virDomainPtr domain, virTypedParameterPtr* params, int* nparams, unsigned int flags);
int virDomainGetSchedulerParameters(virDomainPtr domain, virTypedParameterPtr params, int* nparams);
int virDomainGetSchedulerParametersFlags(virDomainPtr domain, virTypedParameterPtr params, int* nparams, unsigned int flags);

virDomainPtr virDomainMigrate(virDomainPtr domain, virConnectPtr dconn, unsigned long flags, const char* dname, const char* uri,
                              unsigned long bandwidth);
virDomainPtr virDomainMigrate2(virDomainPtr domain, virConnectPtr dconn, const char* dxml, unsigned long flags, const char* dname, const char* uri,
                               unsigned long bandwidth);

virDomainPtr virDomainMigrate3(virDomainPtr domain, virConnectPtr dconn, virTypedParameterPtr params, unsigned int nparams, unsigned int flags);

int virDomainMigrateToURI(virDomainPtr domain, const char* duri, unsigned long flags, const char* dname, unsigned long bandwidth);
int virDomainMigrateToURI2(virDomainPtr domain, const char* dconnuri, const char* miguri, const char* dxml, unsigned long flags, const char* dname,
                           unsigned long bandwidth);
int virDomainMigrateToURI3(virDomainPtr domain, const char* dconnuri, virTypedParameterPtr params, unsigned int nparams, unsigned int flags);

int virDomainOpenChannel(virDomainPtr dom, const char* name, virStreamPtr st, unsigned int flags);
int virDomainOpenConsole(virDomainPtr dom, const char* dev_name, virStreamPtr st, unsigned int flags);

int virDomainOpenGraphics(virDomainPtr dom, unsigned int idx, int fd, unsigned int flags);
int virDomainOpenGraphicsFD(virDomainPtr dom, unsigned int idx, unsigned int flags);

int virDomainPinEmulator(virDomainPtr domain, unsigned char* cpumap, int maplen, unsigned int flags); //
int virDomainPinIOThread(virDomainPtr domain, unsigned int iothread_id, unsigned char* cpumap, int maplen, unsigned int flags);
int virDomainPinVcpu(virDomainPtr domain, unsigned int vcpu, unsigned char* cpumap, int maplen);
int virDomainPinVcpuFlags(virDomainPtr domain, unsigned int vcpu, unsigned char* cpumap, int maplen, unsigned int flags);

int virDomainRestore(virConnectPtr conn, const char* from);
int virDomainRestoreFlags(virConnectPtr conn, const char* from, const char* dxml, unsigned int flags);
int virDomainSave(virDomainPtr domain, const char* to);
int virDomainSaveFlags(virDomainPtr domain, const char* to, const char* dxml, unsigned int flags);
int virDomainSaveImageDefineXML(virConnectPtr conn, const char* file, const char* dxml, unsigned int flags);
char* virDomainSaveImageGetXMLDesc(virConnectPtr conn, const char* file, unsigned int flags);

char* virDomainScreenshot(virDomainPtr domain, virStreamPtr stream, unsigned int screen, unsigned int flags);
int virDomainSendKey(virDomainPtr domain, unsigned int codeset, unsigned int holdtime, unsigned int* keycodes, int nkeycodes, unsigned int flags);
int virDomainSendProcessSignal(virDomainPtr domain, long long pid_value, unsigned int signum, unsigned int flags);
int virDomainSetBlkioParameters(virDomainPtr domain, virTypedParameterPtr params, int nparams, unsigned int flags);
int virDomainSetBlockIoTune(virDomainPtr dom, const char* disk, virTypedParameterPtr params, int nparams, unsigned int flags);
int virDomainSetBlockThreshold(virDomainPtr domain, const char* dev, unsigned long long threshold, unsigned int flags);
int virDomainSetGuestVcpus(virDomainPtr domain, const char* cpumap, int state, unsigned int flags);
int virDomainSetIOThreadParams(virDomainPtr domain, unsigned int iothread_id, virTypedParameterPtr params, int nparams, unsigned int flags);
int virDomainSetInterfaceParameters(virDomainPtr domain, const char* device, virTypedParameterPtr params, int nparams, unsigned int flags);
int virDomainSetLifecycleAction(virDomainPtr domain, unsigned int type, unsigned int action, unsigned int flags);
int virDomainSetMemoryFlags(virDomainPtr domain, unsigned long memory, unsigned int flags);
int virDomainSetMemoryParameters(virDomainPtr domain, virTypedParameterPtr params, int nparams, unsigned int flags);

int virDomainSetMemoryStatsPeriod(virDomainPtr domain, int period, unsigned int flags);
int virDomainSetMetadata(virDomainPtr domain, int type, const char* metadata, const char* key, const char* uri, unsigned int flags);

int virDomainSetNumaParameters(virDomainPtr domain, virTypedParameterPtr params, int nparams, unsigned int flags);
int virDomainSetPerfEvents(virDomainPtr domain, virTypedParameterPtr params, int nparams, unsigned int flags);
int virDomainSetSchedulerParameters(virDomainPtr domain, virTypedParameterPtr params, int nparams);
int virDomainSetSchedulerParametersFlags(virDomainPtr domain, virTypedParameterPtr params, int nparams, unsigned int flags);

int virDomainSetTime(virDomainPtr dom, long long seconds, unsigned int nseconds, unsigned int flags);
int virDomainSetUserPassword(virDomainPtr dom, const char* user, const char* password, unsigned int flags);
int virDomainSetVcpu(virDomainPtr domain, const char* vcpumap, int state, unsigned int flags);
int virDomainSetVcpus(virDomainPtr domain, unsigned int nvcpus);
int virDomainSetVcpusFlags(virDomainPtr domain, unsigned int nvcpus, unsigned int flags);
int virDomainUpdateDeviceFlags(virDomainPtr domain, const char* xml, unsigned int flags);

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
    enum class DestroyFlag {
        DEFAULT = VIR_DOMAIN_DESTROY_DEFAULT,   /* Default behavior - could lead to data loss!! */
        GRACEFUL = VIR_DOMAIN_DESTROY_GRACEFUL, /* only SIGTERM, no SIGKILL */
    };
    enum class DeviceModifyFlag {
        /* See ModificationImpactFlag for these flags.  */
        CURRENT = VIR_DOMAIN_DEVICE_MODIFY_CURRENT,
        LIVE = VIR_DOMAIN_DEVICE_MODIFY_LIVE,
        CONFIG = VIR_DOMAIN_DEVICE_MODIFY_CONFIG,

        /* Additionally, these flags may be bitwise-OR'd in.  */
        FORCE = VIR_DOMAIN_DEVICE_MODIFY_FORCE, /* Forcibly modify device (ex. force eject a cdrom) */
    };
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
    enum class ModificationImpactFlag {
        CURRENT = VIR_DOMAIN_AFFECT_CURRENT, /* Affect current domain state.  */
        LIVE = VIR_DOMAIN_AFFECT_LIVE,       /* Affect running domain state.  */
        CONFIG = VIR_DOMAIN_AFFECT_CONFIG,   /* Affect persistent domain state.  */
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
    class States : public EnumHelper<States, State> {
        using Base = EnumHelper<States, State>;
        friend Base;
        constexpr static std::array values = {"No State",      "Running", "Blocked", "Paused",
                                              "Shutting down", "Shutoff", "Crashed", "Power Management Suspended"};
    };
    constexpr static States States{};
    class SaveRestoreFlags : public EnumSetHelper<SaveRestoreFlags, SaveRestoreFlag> {
        using Base = EnumSetHelper<SaveRestoreFlags, State>;
        friend Base;
        constexpr static std::array values = {"bypass_cache", "running", "paused"};
    };
    constexpr static SaveRestoreFlags SaveRestoreFlags{};
    enum class UndefineFlags {
        MANAGED_SAVE = VIR_DOMAIN_UNDEFINE_MANAGED_SAVE,             /* Also remove any managed save */
        SNAPSHOTS_METADATA = VIR_DOMAIN_UNDEFINE_SNAPSHOTS_METADATA, /* If last use of domain, then also remove any snapshot metadata */
        NVRAM = VIR_DOMAIN_UNDEFINE_NVRAM,                           /* Also remove any nvram file */
        KEEP_NVRAM = VIR_DOMAIN_UNDEFINE_KEEP_NVRAM,                 /* Keep nvram file */
    };

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

    bool create() noexcept;

    bool create(CreateFlag flags) noexcept;

    // createWithFiles() // Left out

    bool coreDump(std::filesystem::path to, CoreDump::Flag flags) const noexcept;

    bool coreDump(std::filesystem::path to, CoreDump::Format format, CoreDump::Flag flags) const noexcept;

    bool delIOThread(unsigned int iothread_id, ModificationImpactFlag flags) noexcept;

    bool destroy() noexcept;

    bool destroy(DestroyFlag flags) noexcept;

    bool detachDevice(gsl::czstring<> xml) noexcept;

    bool detachDevice(gsl::czstring<> xml, DeviceModifyFlag flags = DeviceModifyFlag::LIVE) noexcept;

    bool detachDeviceAlias(gsl::czstring<> alias, DeviceModifyFlag flags = DeviceModifyFlag::LIVE) noexcept;

    int fsFreeze(gsl::span<gsl::czstring<>> mountpoints) noexcept;

    int fsThaw(gsl::span<gsl::czstring<>> mountpoints) noexcept;

    bool fsTrim(gsl::czstring<> mountpoint, unsigned long long minimum) noexcept;

    [[nodiscard]] bool getAutostart() const noexcept;

    [[nodiscard]] Connection getConnect() const noexcept;

    [[nodiscard]] std::optional<virDomainControlInfo> getControlInfo() const noexcept;

    [[nodiscard]] auto getDiskErrors() const noexcept;

    [[nodiscard]] std::vector<DiskError> extractDiskErrors() const;

    // [[nodiscard]] int getEmulatorPinInfo(unsigned char* cpumap, int maplen, unsigned int flags); // TODO figure out the intended way of using
    // this

    [[nodiscard]] auto getFSInfo() const noexcept;

    [[nodiscard]] std::vector<FSInfo> extractFSInfo() const;

    [[nodiscard]] UniqueZstring getHostname() const noexcept;

    [[nodiscard]] std::string extractHostname() const noexcept;

    [[nodiscard]] unsigned getID() const noexcept;

    [[nodiscard]] auto getIOThreadInfo(ModificationImpactFlag flags) const noexcept;

    [[nodiscard]] auto extractIOThreadInfo(ModificationImpactFlag flags) const -> std::vector<heavy::IOThreadInfo>;

    [[nodiscard]] Info getInfo() const noexcept;

    [[nodiscard]] std::optional<JobInfo> getJobInfo() const noexcept;

    [[nodiscard]] int getMaxVcpus() const noexcept;

    [[nodiscard]] UniqueZstring getMetadata(MetadataType type, gsl::czstring<> ns,
                                            ModificationImpactFlag flags = ModificationImpactFlag::CURRENT) const noexcept;

    [[nodiscard]] std::string extractMetadata(MetadataType type, gsl::czstring<> ns,
                                              ModificationImpactFlag flags = ModificationImpactFlag::CURRENT) const;

    [[nodiscard]] gsl::czstring<> getName() const noexcept;

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

    [[nodiscard]] auto getVcpuPinInfo(VCpuFlag flags) -> std::optional<std::vector<unsigned char>>;

    [[nodiscard]] auto getVcpus() const noexcept;

    [[nodiscard]] gsl::czstring<> getXMLDesc(XmlFlag flags = XmlFlag::DEFAULT) const noexcept;

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

    bool managedSave(SaveRestoreFlag flags) noexcept;

    bool managedSaveDefineXML(gsl::czstring<> dxml, SaveRestoreFlag flags) noexcept;

    [[nodiscard]] UniqueZstring managedSaveGetXMLDesc(SaveImageXMLFlag flags) const noexcept;

    [[nodiscard]] std::string managedSaveExtractXMLDesc(SaveImageXMLFlag flags) const noexcept;

    bool managedSaveRemove() noexcept;

    bool memoryPeek(unsigned long long start, gsl::span<unsigned char> buffer, MemoryFlag flags) const noexcept;

    auto memoryStats(unsigned int nr_stats) const noexcept;

    [[nodiscard]] auto migrateGetCompressionCache() const noexcept -> std::optional<unsigned long long>;

    [[nodiscard]] auto migrateGetMaxDowntime() const noexcept -> std::optional<unsigned long long>;

    [[nodiscard]] auto migrateGetMaxSpeed(unsigned int flags) const noexcept -> std::optional<unsigned long>;

    bool migrateSetCompressionCache(unsigned long long cacheSize) noexcept;

    bool migrateSetMaxDowntime(unsigned long long downtime) noexcept;

    bool migrateSetMaxSpeed(unsigned long bandwidth, unsigned int flags) noexcept;

    bool migrateStartPostCopy(unsigned int flags) noexcept;

    bool setMaxMemory(unsigned long);

    bool setMemory(unsigned long);

    void reboot(ShutdownFlag flags = ShutdownFlag::DEFAULT);

    void reset();

    bool rename(gsl::czstring<>);

    bool resume() noexcept;

    bool setAutoStart(bool);
    bool shutdown() noexcept;

    bool shutdown(ShutdownFlag flags) noexcept;

    void suspend();

    bool undefine() noexcept;

    bool undefine(UndefineFlags) noexcept;

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
    constexpr gsl::span<const unsigned char> cpumap() const noexcept { return {Base::cpumap, Base::cpumaplen}; }
    constexpr gsl::span<unsigned char> cpumap() noexcept { return {Base::cpumap, Base::cpumaplen}; }
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
    constexpr gsl::span<const unsigned char> cpumap() const noexcept { return {m_cpumap.data(), static_cast<long>(m_cpumap.size())}; }
    constexpr gsl::span<unsigned char> cpumap() noexcept { return {m_cpumap.data(), static_cast<long>(m_cpumap.size())}; }
};

[[nodiscard]] constexpr inline Domain::CoreDump::Flag operator|(Domain::CoreDump::Flag lhs, Domain::CoreDump::Flag rhs) noexcept;
[[nodiscard]] constexpr inline Domain::GetAllDomainStatsFlag operator|(Domain::GetAllDomainStatsFlag lhs, Domain::GetAllDomainStatsFlag rhs) noexcept;
constexpr inline Domain::GetAllDomainStatsFlag operator|=(Domain::GetAllDomainStatsFlag& lhs,
                                                                        Domain::GetAllDomainStatsFlag rhs) noexcept;
[[nodiscard]] constexpr inline Domain::ShutdownFlag operator|(Domain::ShutdownFlag lhs, Domain::ShutdownFlag rhs) noexcept;
constexpr inline Domain::ShutdownFlag operator|=(Domain::ShutdownFlag& lhs, Domain::ShutdownFlag rhs) noexcept;
[[nodiscard]] constexpr inline Domain::StatsType operator|(Domain::StatsType lhs, Domain::StatsType rhs) noexcept;
constexpr inline Domain::StatsType operator|=(Domain::StatsType& lhs, Domain::StatsType rhs) noexcept;
[[nodiscard]] constexpr inline Domain::ModificationImpactFlag operator|(Domain::ModificationImpactFlag lhs,
                                                                        Domain::ModificationImpactFlag rhs) noexcept;
[[nodiscard]] constexpr inline Domain::VCpuFlag operator|(Domain::VCpuFlag lhs, Domain::VCpuFlag rhs) noexcept;
constexpr inline Domain::VCpuFlag operator|=(Domain::VCpuFlag& lhs, Domain::VCpuFlag rhs) noexcept;
[[nodiscard]] constexpr inline Domain::Stats::Types operator|(Domain::Stats::Types lhs, Domain::Stats::Types rhs) noexcept;
} // namespace virt

constexpr unsigned to_integral(virt::Domain::SaveRestoreFlag f) noexcept { return virt::impl_to_integral(f); }