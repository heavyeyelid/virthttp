#ifndef VIRTPP_ENUM_STORAGE_POOLCREATEFLAG_HPP
#define VIRTPP_ENUM_STORAGE_POOLCREATEFLAG_HPP

#include "../../StoragePool.hpp"
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-storage.h>

namespace virt {
class StoragePool::CreateFlag : private VirtEnumStorage<virStoragePoolDeleteFlags>,
                                public VirtEnumBase<CreateFlag>,
                                public EnumSetHelper<CreateFlag> {
    friend VirtEnumBase<CreateFlag>;
    friend EnumSetHelper<CreateFlag>;
    enum class Underlying {
        NORMAL = VIR_STORAGE_POOL_CREATE_NORMAL,

        /* Create the pool and perform pool build without any flags */
        WITH_BUILD = VIR_STORAGE_POOL_CREATE_WITH_BUILD,

        /* Create the pool and perform pool build using the
         * VIR_STORAGE_POOL_BUILD_OVERWRITE flag. This is mutually
         * exclusive to VIR_STORAGE_POOL_CREATE_WITH_BUILD_NO_OVERWRITE */
        WITH_BUILD_OVERWRITE = VIR_STORAGE_POOL_CREATE_WITH_BUILD_OVERWRITE,

        /* Create the pool and perform pool build using the
         * VIR_STORAGE_POOL_BUILD_NO_OVERWRITE flag. This is mutually
         * exclusive to VIR_STORAGE_POOL_CREATE_WITH_BUILD_OVERWRITE */
        WITH_BUILD_NO_OVERWRITE = VIR_STORAGE_POOL_CREATE_WITH_BUILD_NO_OVERWRITE,
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"with_build", "with_build_overwrite", "with_build_no_overwrite"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto NORMAL = Underlying::NORMAL;
    constexpr static auto WITH_BUILD = Underlying::WITH_BUILD;
    constexpr static auto WITH_BUILD_OVERWRITE = Underlying::WITH_BUILD_OVERWRITE;
    constexpr static auto WITH_BUILD_NO_OVERWRITE = Underlying::WITH_BUILD_NO_OVERWRITE;
};

} // namespace virt

#endif