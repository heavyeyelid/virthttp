//
// Created by hugo on 5/13/20.
//

#pragma once

#include <libvirt/libvirt-domain.h>

namespace virt::enums::domain {
namespace stats {
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
} // namespace stats

namespace core_dump {
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
} // namespace core_dump

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

namespace state_reason {
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
} // namespace state_reason

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

class SaveRestoreFlag;
} // namespace virt::enums::domain
