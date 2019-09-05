#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::DeviceModifyFlag : private VirtEnumStorage<virDomainDeviceModifyFlags>,
                                 public VirtEnumBase<DeviceModifyFlag>,
                                 public EnumSetHelper<DeviceModifyFlag> {
    friend VirtEnumBase<DeviceModifyFlag>;
    friend EnumSetHelper<DeviceModifyFlag>;
    enum class Underlying {
        /* See ModificationImpactFlag for these flags.  */
        CURRENT = VIR_DOMAIN_DEVICE_MODIFY_CURRENT,
        LIVE = VIR_DOMAIN_DEVICE_MODIFY_LIVE,
        CONFIG = VIR_DOMAIN_DEVICE_MODIFY_CONFIG,

        /* Additionally, these flags may be bitwise-OR'd in.  */
        FORCE = VIR_DOMAIN_DEVICE_MODIFY_FORCE, /* Forcibly modify device (ex. force eject a cdrom) */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"live", "config", "force"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto CURRENT = Underlying::CURRENT;
    constexpr static auto LIVE = Underlying::LIVE;
    constexpr static auto CONFIG = Underlying::CONFIG;
};

}