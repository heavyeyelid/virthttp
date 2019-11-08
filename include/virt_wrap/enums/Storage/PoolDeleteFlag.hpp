#ifndef VIRTPP_ENUM_STORAGE_POOLDELETEFLAG_HPP
#define VIRTPP_ENUM_STORAGE_POOLDELETEFLAG_HPP

#include "../../StoragePool.hpp"
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-storage.h>

namespace virt {
class StoragePool::DeleteFlag : private VirtEnumStorage<virStoragePoolDeleteFlags>,
                                public VirtEnumBase<DeleteFlag>,
                                public EnumSetHelper<DeleteFlag> {
    friend VirtEnumBase<DeleteFlag>;
    friend EnumSetHelper<DeleteFlag>;
    enum class Underlying {
        NORMAL = VIR_STORAGE_POOL_DELETE_NORMAL, /* Delete metadata only    (fast) */
        ZEROED = VIR_STORAGE_POOL_DELETE_ZEROED, /* Clear all data to zeros (slow) */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"zeroed"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto NORMAL = Underlying::NORMAL;
    constexpr static auto ZEROED = Underlying::ZEROED;
};

} // namespace virt

#endif