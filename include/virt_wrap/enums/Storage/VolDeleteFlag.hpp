#ifndef VIRTPP_ENUM_STORAGE_VOLDELETEFLAG_HPP
#define VIRTPP_ENUM_STORAGE_VOLDELETEFLAG_HPP

#include "../../StorageVol.hpp"
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-storage.h>

namespace virt {
class StorageVol::DeleteFlag : private VirtEnumStorage<virStorageVolDeleteFlags>, public VirtEnumBase<DeleteFlag>, public EnumSetHelper<DeleteFlag> {
    friend VirtEnumBase<DeleteFlag>;
    friend EnumSetHelper<DeleteFlag>;
    enum class Underlying {
        NORMAL = VIR_STORAGE_VOL_DELETE_NORMAL,                 /* Delete metadata only    (fast) */
        ZEROED = VIR_STORAGE_VOL_DELETE_ZEROED,                 /* Clear all data to zeros (slow) */
        WITH_SNAPSHOTS = VIR_STORAGE_VOL_DELETE_WITH_SNAPSHOTS, /* Force removal of volume, even if in use */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"zeroed", "with_snapshots"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto NORMAL = Underlying::NORMAL;
    constexpr static auto ZEROED = Underlying::ZEROED;
    constexpr static auto WITH_SNAPSHOTS = Underlying::WITH_SNAPSHOTS;
};

} // namespace virt

#endif