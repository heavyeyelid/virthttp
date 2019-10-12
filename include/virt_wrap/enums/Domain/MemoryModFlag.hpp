#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::MemoryModFlag : private VirtEnumStorage<virDomainMemoryModFlags>,
                              public VirtEnumBase<MemoryModFlag>,
                              public EnumSetHelper<MemoryModFlag> {
    friend VirtEnumBase<MemoryModFlag>;
    friend EnumSetHelper<MemoryModFlag>;
    enum class Underlying {
        /* See virDomainModificationImpact for these flags.  */
        LIVE = VIR_DOMAIN_MEM_LIVE,
        CONFIG = VIR_DOMAIN_MEM_CONFIG,

        /* Additionally, these flags may be bitwise-OR'd in.  */
        MAXIMUM = VIR_DOMAIN_MEM_MAXIMUM, /* affect Max rather than current */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"live", "config", "maximum"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto LIVE = Underlying::LIVE;
    constexpr static auto CONFIG = Underlying::CONFIG;
    constexpr static auto MAXIMUM = Underlying::MAXIMUM;
};

}