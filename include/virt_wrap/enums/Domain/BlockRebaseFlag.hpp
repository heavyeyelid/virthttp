#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::BlockRebaseFlag : private VirtEnumStorage<virDomainBlockRebaseFlags>,
                                public VirtEnumBase<BlockRebaseFlag>,
                                public EnumSetHelper<BlockRebaseFlag> {
    friend VirtEnumBase<BlockRebaseFlag>;
    friend EnumSetHelper<BlockRebaseFlag>;
    enum class Underlying {
        SHALLOW = VIR_DOMAIN_BLOCK_REBASE_SHALLOW,                 /* Limit copy to top of source backing chain */
        REUSE_EXT = VIR_DOMAIN_BLOCK_REBASE_REUSE_EXT,             /* Reuse existing external file for a copy */
        COPY_RAW = VIR_DOMAIN_BLOCK_REBASE_COPY_RAW,               /* Make destination file raw */
        COPY = VIR_DOMAIN_BLOCK_REBASE_COPY,                       /* Start a copy job */
        RELATIVE = VIR_DOMAIN_BLOCK_REBASE_RELATIVE,               /* Keep backing chain referenced using relative names */
        COPY_DEV = VIR_DOMAIN_BLOCK_REBASE_COPY_DEV,               /* Treat destination as block device instead of file */
        BANDWIDTH_BYTES = VIR_DOMAIN_BLOCK_REBASE_BANDWIDTH_BYTES, /* bandwidth in bytes/s instead of MiB/s */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"shallow", "reuse_ext", "copy_raw", "copy", "relative", "copy_dev", "bandwidth_bytes"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto SHALLOW = Underlying::SHALLOW;
    constexpr static auto REUSE_EXT = Underlying::REUSE_EXT;
    constexpr static auto COPY_RAW = Underlying::COPY_RAW;
    constexpr static auto COPY = Underlying::COPY;
    constexpr static auto RELATIVE = Underlying::RELATIVE;
    constexpr static auto COPY_DEV = Underlying::COPY_DEV;
    constexpr static auto BANDWIDTH_BYTES = Underlying::BANDWIDTH_BYTES;
};

}