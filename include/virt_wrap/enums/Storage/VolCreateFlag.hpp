#ifndef VIRTPP_ENUM_STORAGE_VOLCREATEFLAG_HPP
#define VIRTPP_ENUM_STORAGE_VOLCREATEFLAG_HPP

#include "../../StorageVol.hpp"
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-storage.h>

namespace virt {
class StorageVol::CreateFlag : private VirtEnumStorage<virStorageVolCreateFlags>, public VirtEnumBase<CreateFlag>, public EnumSetHelper<CreateFlag> {
    friend VirtEnumBase<CreateFlag>;
    friend EnumSetHelper<CreateFlag>;
    enum class Underlying {
        PREALLOC_METADATA = VIR_STORAGE_VOL_CREATE_PREALLOC_METADATA,
        REFLINK = VIR_STORAGE_VOL_CREATE_REFLINK, /* perform a btrfs lightweight copy */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"prealloc_metadata", "reflink"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto PREALLOC_METADATA = Underlying::PREALLOC_METADATA;
    constexpr static auto REFLINK = Underlying::REFLINK;
};

} // namespace virt

#endif