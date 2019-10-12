#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::BlockJobInfoFlag : private VirtEnumStorage<virDomainBlockJobInfoFlags>,
                                 public VirtEnumBase<BlockJobInfoFlag>,
                                 public EnumSetHelper<BlockJobInfoFlag> {
    friend VirtEnumBase<BlockJobInfoFlag>;
    friend EnumSetHelper<BlockJobInfoFlag>;
    enum class Underlying {
        BANDWIDTH_BYTES = VIR_DOMAIN_BLOCK_JOB_INFO_BANDWIDTH_BYTES, /* bandwidth in bytes/s */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"bandwidth_bytes"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto BANDWIDTH_BYTES = Underlying::BANDWIDTH_BYTES;
};

}