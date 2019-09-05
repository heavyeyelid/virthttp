#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::OpenGraphicsFlag : private VirtEnumStorage<virDomainOpenGraphicsFlags>,
                                 public VirtEnumBase<OpenGraphicsFlag>,
                                 public EnumSetHelper<OpenGraphicsFlag> {
    friend VirtEnumBase<OpenGraphicsFlag>;
    friend EnumSetHelper<OpenGraphicsFlag>;
    enum class Underlying {
        SKIPAUTH = VIR_DOMAIN_OPEN_GRAPHICS_SKIPAUTH,
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"skipauth"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto SKIPAUTH = Underlying::SKIPAUTH;
};

}