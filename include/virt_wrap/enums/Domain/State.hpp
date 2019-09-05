#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::State : private VirtEnumStorage<virDomainState>, public VirtEnumBase<State>, public EnumHelper<State> {
    friend VirtEnumBase<State>;
    friend EnumHelper<State>;
    enum class Underlying {
        NOSTATE = VIR_DOMAIN_NOSTATE,         /* no state */
        RUNNING = VIR_DOMAIN_RUNNING,         /* the domain is running */
        BLOCKED = VIR_DOMAIN_BLOCKED,         /* the domain is blocked on resource */
        PAUSED = VIR_DOMAIN_PAUSED,           /* the domain is paused by user */
        SHUTDOWN = VIR_DOMAIN_SHUTDOWN,       /* the domain is being shut down */
        SHUTOFF = VIR_DOMAIN_SHUTOFF,         /* the domain is shut off */
        CRASHED = VIR_DOMAIN_CRASHED,         /* the domain is crashed */
        PMSUSPENDED = VIR_DOMAIN_PMSUSPENDED, /* the domain is suspended by guest power management */
        ENUM_END
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"No State",      "Running", "Blocked", "Paused",
                                          "Shutting down", "Shutoff", "Crashed", "Power Management Suspended"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto NOSTATE = Underlying::NOSTATE;
    constexpr static auto RUNNING = Underlying::RUNNING;
    constexpr static auto BLOCKED = Underlying::BLOCKED;
    constexpr static auto PAUSED = Underlying::PAUSED;
    constexpr static auto SHUTDOWN = Underlying::SHUTDOWN;
    constexpr static auto SHUTOFF = Underlying::SHUTOFF;
    constexpr static auto CRASHED = Underlying::CRASHED;
    constexpr static auto PMSUSPENDED = Underlying::PMSUSPENDED;
    constexpr static auto ENUM_END = Underlying::ENUM_END;
};

} // namespace virt
