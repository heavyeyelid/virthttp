#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/enums/Domain/ModificationImpactFlag.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt::enums::domain {
namespace detail {

class VCpuFlag : private VirtEnumStorage<virDomainVcpuFlags>, public VirtEnumBase<VCpuFlag>, public EnumSetHelper<VCpuFlag> {
    friend VirtEnumBase<VCpuFlag>;
    friend EnumSetHelper<VCpuFlag>;
    enum class Underlying {
        DEFAULT = 0,
        MAXIMUM = VIR_DOMAIN_VCPU_MAXIMUM,           /* Max rather than current count */
        GUEST = VIR_DOMAIN_VCPU_GUEST,               /* Modify state of the cpu in the guest */
        HOTPLUGGABLE = VIR_DOMAIN_VCPU_HOTPLUGGABLE, /* Make vcpus added hot(un)pluggable */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"", "", "maximum", "guest", "hotpluggable"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto DEFAULT = Underlying::DEFAULT;
    constexpr static auto MAXIMUM = Underlying::MAXIMUM;
    constexpr static auto GUEST = Underlying::GUEST;
    constexpr static auto HOTPLUGGABLE = Underlying::HOTPLUGGABLE;
};

} // namespace detail

class VCpuFlag : public EnumSetTie<ModificationImpactFlag, detail::VCpuFlag> {};

} // namespace virt::enums::domain