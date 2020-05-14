//
// Created by hugo on 5/13/20.
//

#pragma once
#include <libvirt/libvirt.h>

namespace virt::enums::connection {
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

enum class CredentialType : int {
    AUTHNAME = VIR_CRED_AUTHNAME,     // Identify to authorize as
    CNONCE = VIR_CRED_CNONCE,         // Client supplies a nonce
    ECHOPROMPT = VIR_CRED_ECHOPROMPT, // Challenge response
    EXTERNAL = VIR_CRED_EXTERNAL,     // Externally managed credential More may be
    // added - expect the unexpected
    LANGUAGE = VIR_CRED_LANGUAGE,         // RFC 1766 languages, comma separated
    NOECHOPROMPT = VIR_CRED_NOECHOPROMPT, // Challenge response
    PASSPHRASE = VIR_CRED_PASSPHRASE,     // Passphrase secret
    REALM = VIR_CRED_REALM,               // Authentication realm
    USERNAME = VIR_CRED_USERNAME,         // Identity to act as
};

enum class Flags : unsigned {
    RO = VIR_CONNECT_RO,                 /* A readonly connection */
    NO_ALIASES = VIR_CONNECT_NO_ALIASES, /* Don't try to resolve URI aliases */
};

namespace list {
namespace devices {
enum class Flags : unsigned {
    DEFAULT = 0,
    SYSTEM = VIR_CONNECT_LIST_NODE_DEVICES_CAP_SYSTEM,
    PCI_DEV = VIR_CONNECT_LIST_NODE_DEVICES_CAP_PCI_DEV,
    USB_DEV = VIR_CONNECT_LIST_NODE_DEVICES_CAP_USB_DEV,
    USB_IFACE = VIR_CONNECT_LIST_NODE_DEVICES_CAP_USB_INTERFACE,
    NETWORK = VIR_CONNECT_LIST_NODE_DEVICES_CAP_NET,
    SCSI_HOST = VIR_CONNECT_LIST_NODE_DEVICES_CAP_SCSI_HOST,
    SCSI_TARGET = VIR_CONNECT_LIST_NODE_DEVICES_CAP_SCSI_TARGET,
    SCSI_DEV = VIR_CONNECT_LIST_NODE_DEVICES_CAP_SCSI,
    STORAGE = VIR_CONNECT_LIST_NODE_DEVICES_CAP_STORAGE,
    FC_HOST = VIR_CONNECT_LIST_NODE_DEVICES_CAP_FC_HOST,
    VPORTS = VIR_CONNECT_LIST_NODE_DEVICES_CAP_VPORTS,
    SCSI_GENERIC = VIR_CONNECT_LIST_NODE_DEVICES_CAP_SCSI_GENERIC,
    DRM = VIR_CONNECT_LIST_NODE_DEVICES_CAP_DRM,
    MEDIATED_CAPABLE = VIR_CONNECT_LIST_NODE_DEVICES_CAP_MDEV_TYPES,
    MEDIATED = VIR_CONNECT_LIST_NODE_DEVICES_CAP_MDEV,
    CCW = VIR_CONNECT_LIST_NODE_DEVICES_CAP_CCW_DEV,
};
}
namespace networks {
enum class Flag : unsigned {
    DEFAULT = 0,

    INACTIVE = VIR_CONNECT_LIST_NETWORKS_INACTIVE,
    ACTIVE = VIR_CONNECT_LIST_NETWORKS_ACTIVE,

    PERSISTENT = VIR_CONNECT_LIST_NETWORKS_PERSISTENT,
    TRANSIENT = VIR_CONNECT_LIST_NETWORKS_TRANSIENT,

    AUTOSTART = VIR_CONNECT_LIST_NETWORKS_AUTOSTART,
    NO_AUTOSTART = VIR_CONNECT_LIST_NETWORKS_NO_AUTOSTART,
};
[[nodiscard]] constexpr Flag operator|(Flag lhs, Flag rhs) noexcept { return Flag{to_integral(lhs) | to_integral(rhs)}; }
constexpr Flag& operator|=(Flag& lhs, Flag rhs) noexcept { return lhs = Flag{to_integral(lhs) | to_integral(rhs)}; }
} // namespace networks
} // namespace list
namespace get_all_domains::stats {
enum class Flags : unsigned {
    ACTIVE = VIR_CONNECT_GET_ALL_DOMAINS_STATS_ACTIVE,
    INACTIVE = VIR_CONNECT_GET_ALL_DOMAINS_STATS_INACTIVE,

    PERSISTENT = VIR_CONNECT_LIST_DOMAINS_PERSISTENT,
    TRANSIENT = VIR_CONNECT_LIST_DOMAINS_TRANSIENT,

    RUNNING = VIR_CONNECT_LIST_DOMAINS_RUNNING,
    PAUSED = VIR_CONNECT_GET_ALL_DOMAINS_STATS_PAUSED,
    SHUTOFF = VIR_CONNECT_LIST_DOMAINS_SHUTOFF,
    OTHER = VIR_CONNECT_GET_ALL_DOMAINS_STATS_OTHER,

    NOWAIT = VIR_CONNECT_GET_ALL_DOMAINS_STATS_NOWAIT, // report statistics that can be obtained immediately without any
    // blocking
    BACKING = VIR_CONNECT_GET_ALL_DOMAINS_STATS_BACKING,             // include backing chain for block stats
    ENFORCE_STATS = VIR_CONNECT_GET_ALL_DOMAINS_STATS_ENFORCE_STATS, // enforce requested stats
};
}

[[nodiscard]] constexpr inline Flags operator|(Flags lhs, Flags rhs) noexcept;

[[nodiscard]] constexpr inline get_all_domains::stats::Flags operator|(get_all_domains::stats::Flags lhs, get_all_domains::stats::Flags rhs) noexcept;

constexpr inline list::devices::Flags operator|(list::devices::Flags lhs, list::devices::Flags rhs) noexcept;

constexpr inline Flags operator|(Flags lhs, Flags rhs) noexcept { return Flags(to_integral(lhs) | to_integral(rhs)); }

[[nodiscard]] constexpr inline get_all_domains::stats::Flags operator|(get_all_domains::stats::Flags lhs,
                                                                       get_all_domains::stats::Flags rhs) noexcept {
    return get_all_domains::stats::Flags(to_integral(lhs) | to_integral(rhs));
}

constexpr inline list::devices::Flags operator|(list::devices::Flags lhs, list::devices::Flags rhs) noexcept {
    return list::devices::Flags{to_integral(lhs) | to_integral(rhs)};
}

} // namespace virt::enums::connection