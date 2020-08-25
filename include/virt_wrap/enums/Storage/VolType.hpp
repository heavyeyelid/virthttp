#ifndef VIRTPP_ENUM_STORAGE_VOLTYPE_HPP
#define VIRTPP_ENUM_STORAGE_VOLTYPE_HPP

#include "../../StorageVol.hpp"
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-storage.h>

namespace virt {
class StorageVol::Type : private VirtEnumStorage<virStorageVolType>, public VirtEnumBase<Type>, public EnumHelper<Type> {
    friend VirtEnumBase<Type>;
    friend EnumHelper<Type>;
    enum class Underlying {
        FILE = VIR_STORAGE_VOL_FILE,       /* Regular file based volumes */
        BLOCK = VIR_STORAGE_VOL_BLOCK,     /* Block based volumes */
        DIR = VIR_STORAGE_VOL_DIR,         /* Directory-passthrough based volume */
        NETWORK = VIR_STORAGE_VOL_NETWORK, /* Network volumes like RBD (RADOS Block Device) */
        NETDIR = VIR_STORAGE_VOL_NETDIR,   /* Network accessible directory that can contain other network volumes */
        PLOOP = VIR_STORAGE_VOL_PLOOP,     /* Ploop based volumes */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"file", "block", "dir", "network", "netdir", "ploop"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto FILE = Underlying::FILE;
    constexpr static auto BLOCK = Underlying::BLOCK;
    constexpr static auto DIR = Underlying::DIR;
    constexpr static auto NETWORK = Underlying::NETWORK;
    constexpr static auto NETDIR = Underlying::NETDIR;
    constexpr static auto PLOOP = Underlying::PLOOP;
};

} // namespace virt

#endif