#ifndef VIRTPP_ENUM_STORAGE_VOLUPLOADFLAG_HPP
#define VIRTPP_ENUM_STORAGE_VOLUPLOADFLAG_HPP

#include "../../StorageVol.hpp"
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-storage.h>

namespace virt {
class StorageVol::UploadFlag : private VirtEnumStorage<virStorageVolUploadFlags>, public VirtEnumBase<UploadFlag>, public EnumSetHelper<UploadFlag> {
    friend VirtEnumBase<UploadFlag>;
    friend EnumSetHelper<UploadFlag>;
    enum class Underlying {
        SPARSE_STREAM = VIR_STORAGE_VOL_UPLOAD_SPARSE_STREAM, /* Use sparse stream */
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