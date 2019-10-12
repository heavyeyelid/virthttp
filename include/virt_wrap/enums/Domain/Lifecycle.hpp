#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::Lifecycle : private VirtEnumStorage<virDomainLifecycle>, public VirtEnumBase<Lifecycle>, public EnumHelper<Lifecycle> {
    friend VirtEnumBase<Lifecycle>;
    friend EnumHelper<Lifecycle>;
    enum class Underlying {
        POWEROFF = VIR_DOMAIN_LIFECYCLE_POWEROFF,
        REBOOT = VIR_DOMAIN_LIFECYCLE_REBOOT,
        CRASH = VIR_DOMAIN_LIFECYCLE_CRASH,
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"poweroff", "reboot", "crash"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto POWEROFF = Underlying::POWEROFF;
    constexpr static auto REBOOT = Underlying::REBOOT;
    constexpr static auto CRASH = Underlying::CRASH;
};

}