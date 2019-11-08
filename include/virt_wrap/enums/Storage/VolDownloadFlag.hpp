#ifndef VIRTPP_ENUM_STORAGE_VOLDOWNLOADFLAG_HPP
#define VIRTPP_ENUM_STORAGE_VOLDOWNLOADFLAG_HPP

#include "../../StorageVol.hpp"
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-storage.h>

namespace virt {
class StorageVol::DownloadFlag : private VirtEnumStorage<virStorageVolDownloadFlags>,
                                 public VirtEnumBase<DownloadFlag>,
                                 public EnumSetHelper<DownloadFlag> {
    friend VirtEnumBase<DownloadFlag>;
    friend EnumSetHelper<DownloadFlag>;
    enum class Underlying {
        SPARSE_STREAM = VIR_STORAGE_VOL_DOWNLOAD_SPARSE_STREAM, /* Use sparse stream */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"sparse_stream"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto SPARSE_STREAM = Underlying::SPARSE_STREAM;
};

} // namespace virt

#endif