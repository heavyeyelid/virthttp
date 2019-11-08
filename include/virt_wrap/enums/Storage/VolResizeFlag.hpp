#ifndef VIRTPP_ENUM_STORAGE_VOLRESIZEFLAG_HPP
#define VIRTPP_ENUM_STORAGE_VOLRESIZEFLAG_HPP

#include "../../StorageVol.hpp"
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-storage.h>

namespace virt {
class StorageVol::ResizeFlag : private VirtEnumStorage<virStorageVolResizeFlags>, public VirtEnumBase<ResizeFlag>, public EnumSetHelper<ResizeFlag> {
    friend VirtEnumBase<ResizeFlag>;
    friend EnumSetHelper<ResizeFlag>;
    enum class Underlying {
        ALLOCATE = VIR_STORAGE_VOL_RESIZE_ALLOCATE, /* force allocation of new size */
        DELTA = VIR_STORAGE_VOL_RESIZE_DELTA,       /* size is relative to current */
        SHRINK = VIR_STORAGE_VOL_RESIZE_SHRINK,     /* allow decrease in capacity */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"allocate", "delta", "shrink"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto ALLOCATE = Underlying::ALLOCATE;
    constexpr static auto DELTA = Underlying::DELTA;
    constexpr static auto SHRINK = Underlying::SHRINK;
};

} // namespace virt

#endif