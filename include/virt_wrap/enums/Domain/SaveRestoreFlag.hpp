#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/enums/Domain/Decls.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt::enums::domain {

class SaveRestoreFlag : private VirtEnumStorage<unsigned>,
                        public VirtCustomEnum,
                        public VirtEnumBase<SaveRestoreFlag>,
                        public EnumSetHelper<SaveRestoreFlag> {
    // FIXME Redesign and fix all this mess
    friend VirtCustomEnum;
    friend VirtEnumBase<SaveRestoreFlag>;
    friend EnumSetHelper<SaveRestoreFlag>;

    struct EBase {};
    struct DEFAULT_t : EBase {
        constexpr static auto value = 0;
    };
    struct BYPASS_CACHE_t {
        constexpr static auto value = VIR_DOMAIN_SAVE_BYPASS_CACHE;
    };
    struct RUNNING_t {
        constexpr static auto value = VIR_DOMAIN_SAVE_RUNNING;
    };
    struct PAUSED_t {
        constexpr static auto value = VIR_DOMAIN_SAVE_PAUSED;
    };

    friend constexpr SaveRestoreFlag operator|(DEFAULT_t, BYPASS_CACHE_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(DEFAULT_t, RUNNING_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(DEFAULT_t, PAUSED_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(BYPASS_CACHE_t, DEFAULT_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(BYPASS_CACHE_t, RUNNING_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(BYPASS_CACHE_t, PAUSED_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(RUNNING_t, DEFAULT_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(RUNNING_t, BYPASS_CACHE_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(PAUSED_t, DEFAULT_t) noexcept;
    friend constexpr SaveRestoreFlag operator|(PAUSED_t, BYPASS_CACHE_t) noexcept;

    friend constexpr unsigned impl_to_integral(SaveRestoreFlag f) noexcept { return f.underlying; }

    constexpr static std::array values = {"bypass_cache", "running", "paused"};

  public:
    constexpr static auto DEFAULT = DEFAULT_t{};
    constexpr static auto BYPASS_CACHE = BYPASS_CACHE_t{};
    constexpr static auto RUNNING = RUNNING_t{};
    constexpr static auto PAUSED = PAUSED_t{};

    using VirtEnumBase::VirtEnumBase;
    constexpr explicit SaveRestoreFlag(unsigned u) noexcept { underlying = u; }
    constexpr SaveRestoreFlag(virDomainSaveRestoreFlags u) noexcept { underlying = u; }
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<EBase, T>>> constexpr explicit SaveRestoreFlag(T) noexcept {
        underlying = T::value;
    }
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<EBase, T>>> constexpr SaveRestoreFlag& operator=(T) noexcept {
        underlying = T::value;
        return *this;
    }

    constexpr SaveRestoreFlag& operator|=(DEFAULT_t) noexcept {
        underlying |= 0u;
        return *this;
    }
    constexpr SaveRestoreFlag& operator|=(BYPASS_CACHE_t) noexcept {
        underlying |= VIR_DOMAIN_SAVE_BYPASS_CACHE;
        return *this;
    }
    constexpr SaveRestoreFlag& operator|=(RUNNING_t) {
        underlying &= ~VIR_DOMAIN_SAVE_PAUSED;
        underlying |= VIR_DOMAIN_SAVE_RUNNING;
        return *this;
    }
    constexpr SaveRestoreFlag& operator|=(PAUSED_t) {
        underlying &= ~VIR_DOMAIN_SAVE_RUNNING;
        underlying |= VIR_DOMAIN_SAVE_PAUSED;
        return *this;
    }

    constexpr SaveRestoreFlag operator|(SaveRestoreFlag f) const noexcept;
    constexpr SaveRestoreFlag operator|(DEFAULT_t) const noexcept { return *this; }
    constexpr SaveRestoreFlag operator|(BYPASS_CACHE_t) const noexcept {
        return virDomainSaveRestoreFlags(underlying | VIR_DOMAIN_SAVE_BYPASS_CACHE);
    }
    constexpr SaveRestoreFlag operator|(RUNNING_t) const noexcept {
        return virDomainSaveRestoreFlags((underlying & ~VIR_DOMAIN_SAVE_PAUSED) | VIR_DOMAIN_SAVE_RUNNING);
    }
    constexpr SaveRestoreFlag operator|(PAUSED_t) const noexcept {
        return virDomainSaveRestoreFlags((underlying & ~VIR_DOMAIN_SAVE_RUNNING) | VIR_DOMAIN_SAVE_PAUSED);
    }

    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
};

constexpr domain::SaveRestoreFlag operator|(domain::SaveRestoreFlag::DEFAULT_t, domain::SaveRestoreFlag::BYPASS_CACHE_t) noexcept {
    return {VIR_DOMAIN_SAVE_BYPASS_CACHE};
}
constexpr domain::SaveRestoreFlag operator|(domain::SaveRestoreFlag::DEFAULT_t, domain::SaveRestoreFlag::RUNNING_t) noexcept {
    return {VIR_DOMAIN_SAVE_RUNNING};
}
constexpr domain::SaveRestoreFlag operator|(domain::SaveRestoreFlag::DEFAULT_t, domain::SaveRestoreFlag::PAUSED_t) noexcept {
    return {VIR_DOMAIN_SAVE_PAUSED};
}
constexpr domain::SaveRestoreFlag operator|(domain::SaveRestoreFlag::BYPASS_CACHE_t, domain::SaveRestoreFlag::DEFAULT_t) noexcept {
    return {VIR_DOMAIN_SAVE_BYPASS_CACHE};
}
constexpr domain::SaveRestoreFlag operator|(domain::SaveRestoreFlag::BYPASS_CACHE_t, domain::SaveRestoreFlag::RUNNING_t) noexcept {
    return virDomainSaveRestoreFlags(VIR_DOMAIN_SAVE_BYPASS_CACHE | VIR_DOMAIN_SAVE_RUNNING);
}
constexpr domain::SaveRestoreFlag operator|(domain::SaveRestoreFlag::BYPASS_CACHE_t, domain::SaveRestoreFlag::PAUSED_t) noexcept {
    return virDomainSaveRestoreFlags(VIR_DOMAIN_SAVE_BYPASS_CACHE | VIR_DOMAIN_SAVE_PAUSED);
}
constexpr domain::SaveRestoreFlag operator|(domain::SaveRestoreFlag::RUNNING_t, domain::SaveRestoreFlag::DEFAULT_t) noexcept {
    return {VIR_DOMAIN_SAVE_RUNNING};
}
constexpr domain::SaveRestoreFlag operator|(domain::SaveRestoreFlag::RUNNING_t, domain::SaveRestoreFlag::BYPASS_CACHE_t) noexcept {
    return virDomainSaveRestoreFlags(VIR_DOMAIN_SAVE_BYPASS_CACHE | VIR_DOMAIN_SAVE_RUNNING);
}
constexpr domain::SaveRestoreFlag operator|(domain::SaveRestoreFlag::PAUSED_t, domain::SaveRestoreFlag::DEFAULT_t) noexcept {
    return {VIR_DOMAIN_SAVE_PAUSED};
}
constexpr domain::SaveRestoreFlag operator|(domain::SaveRestoreFlag::PAUSED_t, domain::SaveRestoreFlag::BYPASS_CACHE_t) noexcept {
    return virDomainSaveRestoreFlags(VIR_DOMAIN_SAVE_BYPASS_CACHE | VIR_DOMAIN_SAVE_PAUSED);
}

constexpr unsigned to_integral(SaveRestoreFlag f) noexcept { return impl_to_integral(f); }

} // namespace virt::enums