#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::ChannelFlag : private VirtEnumStorage<virDomainChannelFlags>, public VirtEnumBase<ChannelFlag>, public EnumSetHelper<ChannelFlag> {
    friend VirtEnumBase<ChannelFlag>;
    friend EnumSetHelper<ChannelFlag>;
    enum class Underlying {
        FORCE = VIR_DOMAIN_CHANNEL_FORCE, /* abort a (possibly) active channel connection to force a new connection */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"force"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto FORCE = Underlying::FORCE;
};

}