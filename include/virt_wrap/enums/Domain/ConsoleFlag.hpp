#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::ConsoleFlag : private VirtEnumStorage<virDomainConsoleFlags>, public VirtEnumBase<ConsoleFlag>, public EnumSetHelper<ConsoleFlag> {
    friend VirtEnumBase<ConsoleFlag>;
    friend EnumSetHelper<ConsoleFlag>;
    enum class Underlying {
        FORCE = VIR_DOMAIN_CONSOLE_FORCE, /* abort a (possibly) active console connection to force a new connection */
        SAFE = VIR_DOMAIN_CONSOLE_SAFE,   /* check if the console driver supports safe console operations */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"force", "safe"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto FORCE = Underlying::FORCE;
    constexpr static auto SAFE = Underlying::SAFE;
};

}