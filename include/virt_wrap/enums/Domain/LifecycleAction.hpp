#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::LifecycleAction : private VirtEnumStorage<virDomainLifecycleAction>,
                                public VirtEnumBase<LifecycleAction>,
                                public EnumHelper<LifecycleAction> {
    friend VirtEnumBase<LifecycleAction>;
    friend EnumHelper<LifecycleAction>;
    enum class Underlying {
        DESTROY = VIR_DOMAIN_LIFECYCLE_ACTION_DESTROY,
        RESTART = VIR_DOMAIN_LIFECYCLE_ACTION_RESTART,
        RESTART_RENAME = VIR_DOMAIN_LIFECYCLE_ACTION_RESTART_RENAME,
        PRESERVE = VIR_DOMAIN_LIFECYCLE_ACTION_PRESERVE,
        COREDUMP_DESTROY = VIR_DOMAIN_LIFECYCLE_ACTION_COREDUMP_DESTROY,
        COREDUMP_RESTART = VIR_DOMAIN_LIFECYCLE_ACTION_COREDUMP_RESTART,
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"destroy", "restart", "restart_rename", "preserve", "coredump_destroy", "coredump_restart"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto DESTROY = Underlying::DESTROY;
    constexpr static auto RESTART = Underlying::RESTART;
    constexpr static auto RESTART_RENAME = Underlying::RESTART_RENAME;
    constexpr static auto PRESERVE = Underlying::PRESERVE;
    constexpr static auto COREDUMP_DESTROY = Underlying::COREDUMP_DESTROY;
    constexpr static auto COREDUMP_RESTART = Underlying::COREDUMP_RESTART;
};

}