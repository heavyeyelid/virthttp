#ifndef VIRTPP_ENUM_STORAGE_VOLINFOFLAG_HPP
#define VIRTPP_ENUM_STORAGE_VOLINFOFLAG_HPP

#include "../../StorageVol.hpp"
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-storage.h>

namespace virt {
class StorageVol::InfoFlag : private VirtEnumStorage<virStorageVolInfoFlags>, public VirtEnumBase<InfoFlag>, public EnumSetHelper<InfoFlag> {
    friend VirtEnumBase<InfoFlag>;
    friend EnumSetHelper<InfoFlag>;
    enum class Underlying {
        USE_ALLOCATION = VIR_STORAGE_VOL_USE_ALLOCATION,
        GET_PHYSICAL = VIR_STORAGE_VOL_GET_PHYSICAL, /* Return the physical size in allocation */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"get_physical"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto USE_ALLOCATION = Underlying::USE_ALLOCATION;
    constexpr static auto GET_PHYSICAL = Underlying::GET_PHYSICAL;
};

} // namespace virt

#endif