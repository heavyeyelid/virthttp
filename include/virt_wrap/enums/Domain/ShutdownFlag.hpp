#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::ShutdownFlag : private VirtEnumStorage<virDomainShutdownFlagValues>,
                             public VirtEnumBase<ShutdownFlag>,
                             public EnumSetHelper<ShutdownFlag> {
    friend VirtEnumBase<ShutdownFlag>;
    friend EnumSetHelper<ShutdownFlag>;
    enum class Underlying {
        DEFAULT = VIR_DOMAIN_REBOOT_DEFAULT,               /* hypervisor choice */
        ACPI_POWER_BTN = VIR_DOMAIN_REBOOT_ACPI_POWER_BTN, /* Send ACPI event */
        GUEST_AGENT = VIR_DOMAIN_REBOOT_GUEST_AGENT,       /* Use guest agent */
        INITCTL = VIR_DOMAIN_REBOOT_INITCTL,               /* Use initctl */
        SIGNAL = VIR_DOMAIN_REBOOT_SIGNAL,                 /* Send a signal */
        PARAVIRT = VIR_DOMAIN_REBOOT_PARAVIRT,             /* Use paravirt guest control */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"acpi_power_btn", "guest_agent", "initctl", "signal", "paravirt"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto DEFAULT = Underlying::DEFAULT;
    constexpr static auto ACPI_POWER_BTN = Underlying::ACPI_POWER_BTN;
    constexpr static auto GUEST_AGENT = Underlying::GUEST_AGENT;
    constexpr static auto INITCTL = Underlying::INITCTL;
    constexpr static auto SIGNAL = Underlying::SIGNAL;
    constexpr static auto PARAVIRT = Underlying::PARAVIRT;
};

} // namespace virt
