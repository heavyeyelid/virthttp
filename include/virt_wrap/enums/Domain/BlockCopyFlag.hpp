#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {
class Domain::BlockCopyFlag : private VirtEnumStorage<virDomainBlockCopyFlags>,
                              public VirtEnumBase<BlockCopyFlag>,
                              public EnumSetHelper<BlockCopyFlag> {
    friend VirtEnumBase<BlockCopyFlag>;
    friend EnumSetHelper<BlockCopyFlag>;
    enum class Underlying {
        SHALLOW = VIR_DOMAIN_BLOCK_COPY_SHALLOW,             /* Limit copy to top of source backing chain */
        REUSE_EXT = VIR_DOMAIN_BLOCK_COPY_REUSE_EXT,         /* Reuse existing external file for a copy */
        TRANSIENT_JOB = VIR_DOMAIN_BLOCK_COPY_TRANSIENT_JOB, /* Don't force usage of recoverable job for the copy operation */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"shallow", "reuse_ext", "transient_job"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto SHALLOW = Underlying::SHALLOW;
    constexpr static auto REUSE_EXT = Underlying::REUSE_EXT;
    constexpr static auto TRANSIENT_JOB = Underlying::TRANSIENT_JOB;
};
}