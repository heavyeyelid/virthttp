#ifndef VIRTPP_ENUM_STORAGE_POOLSTATE_HPP
#define VIRTPP_ENUM_STORAGE_POOLSTATE_HPP

#include "../../StoragePool.hpp"
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-storage.h>

namespace virt {
class StoragePool::State : private VirtEnumStorage<virStoragePoolState>, public VirtEnumBase<State>, public EnumHelper<State> {
    friend VirtEnumBase<State>;
    friend EnumHelper<State>;
    enum class Underlying {
        INACTIVE = VIR_STORAGE_POOL_INACTIVE,         /* Not running */
        BUILDING = VIR_STORAGE_POOL_BUILDING,         /* Initializing pool, not available */
        RUNNING = VIR_STORAGE_POOL_RUNNING,           /* Running normally */
        DEGRADED = VIR_STORAGE_POOL_DEGRADED,         /* Running degraded */
        INACCESSIBLE = VIR_STORAGE_POOL_INACCESSIBLE, /* Running, but not accessible */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"inactive", "building", "running", "degraded", "inaccessible"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto INACTIVE = Underlying::INACTIVE;
    constexpr static auto BUILDING = Underlying::BUILDING;
    constexpr static auto RUNNING = Underlying::RUNNING;
    constexpr static auto DEGRADED = Underlying::DEGRADED;
    constexpr static auto INACCESSIBLE = Underlying::INACCESSIBLE;
};

} // namespace virt

#endif