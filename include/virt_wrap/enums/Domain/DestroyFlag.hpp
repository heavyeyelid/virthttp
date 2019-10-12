#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::DestroyFlag : private VirtEnumStorage<virDomainDestroyFlagsValues>,
                            public VirtEnumBase<DestroyFlag>,
                            public EnumSetHelper<DestroyFlag> {
    friend VirtEnumBase<DestroyFlag>;
    friend EnumSetHelper<DestroyFlag>;
    enum class Underlying {
        DEFAULT = VIR_DOMAIN_DESTROY_DEFAULT,   /* Default behavior - could lead to data loss!! */
        GRACEFUL = VIR_DOMAIN_DESTROY_GRACEFUL, /* only SIGTERM, no SIGKILL */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"graceful"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto DEFAULT = Underlying::DEFAULT;
    constexpr static auto GRACEFUL = Underlying::GRACEFUL;
};

}