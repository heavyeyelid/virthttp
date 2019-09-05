#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::ModificationImpactFlag : private VirtEnumStorage<virDomainModificationImpact>,
                                       public VirtEnumBase<ModificationImpactFlag>,
                                       public EnumSetHelper<ModificationImpactFlag> {
    friend VirtEnumBase<ModificationImpactFlag>;
    friend EnumSetHelper<ModificationImpactFlag>;
    enum class Underlying {
        CURRENT = VIR_DOMAIN_AFFECT_CURRENT, /* Affect current domain state.  */
        LIVE = VIR_DOMAIN_AFFECT_LIVE,       /* Affect running domain state.  */
        CONFIG = VIR_DOMAIN_AFFECT_CONFIG,   /* Affect persistent domain state.  */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"live", "config"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto CURRENT = Underlying::CURRENT;
    constexpr static auto LIVE = Underlying::LIVE;
    constexpr static auto CONFIG = Underlying::CONFIG;
};

}