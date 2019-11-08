#ifndef VIRTPP_ENUM_STORAGE_POOLBUILDFLAG_HPP
#define VIRTPP_ENUM_STORAGE_POOLBUILDFLAG_HPP

#include "../../StoragePool.hpp"
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-storage.h>

namespace virt {
class StoragePool::BuildFlag : private VirtEnumStorage<virStoragePoolBuildFlags>, public VirtEnumBase<BuildFlag>, public EnumSetHelper<BuildFlag> {
    friend VirtEnumBase<BuildFlag>;
    friend EnumSetHelper<BuildFlag>;
    enum class Underlying {
        NEW = VIR_STORAGE_POOL_BUILD_NEW,                   /* Regular build from scratch */
        REPAIR = VIR_STORAGE_POOL_BUILD_REPAIR,             /* Delete any files that are now invalid after their contents have been committed */
        RESIZE = VIR_STORAGE_POOL_BUILD_RESIZE,             /* Allow a two-phase commit when top is the active layer */
        OVERWRITE = VIR_STORAGE_POOL_BUILD_OVERWRITE,       /* keep the backing chain referenced using relative names */
        NO_OVERWRITE = VIR_STORAGE_POOL_BUILD_NO_OVERWRITE, /* bandwidth in bytes/s instead of MiB/s */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"repair", "resize", "overwrite", "no_overwrite"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto NEW = Underlying::NEW;
    constexpr static auto REPAIR = Underlying::REPAIR;
    constexpr static auto RESIZE = Underlying::RESIZE;
    constexpr static auto OVERWRITE = Underlying::OVERWRITE;
    constexpr static auto NO_OVERWRITE = Underlying::NO_OVERWRITE;
};

} // namespace virt

#endif