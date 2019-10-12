#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::CreateFlag : private VirtEnumStorage<virDomainCreateFlags>, public VirtEnumBase<CreateFlag>, public EnumSetHelper<CreateFlag> {
    friend VirtEnumBase<CreateFlag>;
    friend EnumSetHelper<CreateFlag>;
    enum class Underlying {
        NONE = VIR_DOMAIN_NONE,
        PAUSED = VIR_DOMAIN_START_PAUSED,
        AUTODESTROY = VIR_DOMAIN_START_AUTODESTROY,
        BYPASS_CACHE = VIR_DOMAIN_START_BYPASS_CACHE,
        FORCE_BOOT = VIR_DOMAIN_START_FORCE_BOOT,
        VALIDATE = VIR_DOMAIN_START_VALIDATE
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"paused", "autodestroy", "bypass_cache", "force_boot", "validate"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto NONE = Underlying::NONE;
    constexpr static auto PAUSED = Underlying::PAUSED;
    constexpr static auto AUTODESTROY = Underlying::AUTODESTROY;
    constexpr static auto BYPASS_CACHE = Underlying::BYPASS_CACHE;
    constexpr static auto FORCE_BOOT = Underlying::FORCE_BOOT;
    constexpr static auto VALIDATE = Underlying::VALIDATE;
};

}