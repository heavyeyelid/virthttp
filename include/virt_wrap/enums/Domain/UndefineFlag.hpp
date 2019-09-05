#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::UndefineFlag : private VirtEnumStorage<virDomainUndefineFlagsValues>,
                             public VirtEnumBase<UndefineFlag>,
                             public EnumSetHelper<UndefineFlag> {
    friend VirtEnumBase<UndefineFlag>;
    friend EnumSetHelper<UndefineFlag>;
    enum class Underlying {
        MANAGED_SAVE = VIR_DOMAIN_UNDEFINE_MANAGED_SAVE,             /* Also remove any managed save */
        SNAPSHOTS_METADATA = VIR_DOMAIN_UNDEFINE_SNAPSHOTS_METADATA, /* If last use of domain, then also remove any snapshot metadata */
        NVRAM = VIR_DOMAIN_UNDEFINE_NVRAM,                           /* Also remove any nvram file */
        KEEP_NVRAM = VIR_DOMAIN_UNDEFINE_KEEP_NVRAM,                 /* Keep nvram file */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"managed_save", "snapshots_metadata", "nvram", "keep_nvram"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto MANAGED_SAVE = Underlying::MANAGED_SAVE;
    constexpr static auto SNAPSHOTS_METADATA = Underlying::SNAPSHOTS_METADATA;
    constexpr static auto NVRAM = Underlying::NVRAM;
    constexpr static auto KEEP_NVRAM = Underlying::KEEP_NVRAM;
};

}