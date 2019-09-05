#pragma once
#include <libvirt/libvirt.h>
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"

namespace virt {

class Connection::List::Domains::Flag : private VirtEnumStorage<virConnectListAllDomainsFlags>,
                                        public VirtEnumBase<Flag>,
                                        public EnumSetHelper<Flag> {
    friend VirtEnumBase<Flag>;
    friend EnumSetHelper<Flag>;
    enum class Underlying : unsigned {
        DEFAULT = 0,
        ACTIVE = VIR_CONNECT_LIST_DOMAINS_ACTIVE,
        INACTIVE = VIR_CONNECT_LIST_DOMAINS_INACTIVE,

        PERSISTENT = VIR_CONNECT_LIST_DOMAINS_PERSISTENT,
        TRANSIENT = VIR_CONNECT_LIST_DOMAINS_TRANSIENT,

        RUNNING = VIR_CONNECT_LIST_DOMAINS_RUNNING,
        PAUSED = VIR_CONNECT_LIST_DOMAINS_PAUSED,
        SHUTOFF = VIR_CONNECT_LIST_DOMAINS_SHUTOFF,
        OTHER = VIR_CONNECT_LIST_DOMAINS_OTHER,

        MANAGEDSAVE = VIR_CONNECT_LIST_DOMAINS_MANAGEDSAVE,
        NO_MANAGEDSAVE = VIR_CONNECT_LIST_DOMAINS_NO_MANAGEDSAVE,

        AUTOSTART = VIR_CONNECT_LIST_DOMAINS_AUTOSTART,
        NO_AUTOSTART = VIR_CONNECT_LIST_DOMAINS_NO_AUTOSTART,

        HAS_SNAPSHOT = VIR_CONNECT_LIST_DOMAINS_HAS_SNAPSHOT,
        NO_SNAPSHOT = VIR_CONNECT_LIST_DOMAINS_NO_SNAPSHOT,
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"active", "inactive",     "persistent",      "transient", "running",      "paused",       "shutoff",
                                          "other",  "managed_save", "no_managed_save", "autostart", "no_autostart", "has_snapshot", "no_snapshot"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto DEFAULT = Underlying::DEFAULT;
    constexpr static auto ACTIVE = Underlying::ACTIVE;
    constexpr static auto INACTIVE = Underlying::INACTIVE;
    constexpr static auto PERSISTENT = Underlying::PERSISTENT;
    constexpr static auto TRANSIENT = Underlying::TRANSIENT;
    constexpr static auto RUNNING = Underlying::RUNNING;
    constexpr static auto PAUSED = Underlying::PAUSED;
    constexpr static auto SHUTOFF = Underlying::SHUTOFF;
    constexpr static auto OTHER = Underlying::OTHER;
    constexpr static auto MANAGEDSAVE = Underlying::MANAGEDSAVE;
    constexpr static auto NO_MANAGEDSAVE = Underlying::NO_MANAGEDSAVE;
    constexpr static auto AUTOSTART = Underlying::AUTOSTART;
    constexpr static auto NO_AUTOSTART = Underlying::NO_AUTOSTART;
    constexpr static auto HAS_SNAPSHOT = Underlying::HAS_SNAPSHOT;
    constexpr static auto NO_SNAPSHOT = Underlying::NO_SNAPSHOT;
};
}