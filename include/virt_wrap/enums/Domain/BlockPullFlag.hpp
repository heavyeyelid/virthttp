#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::BlockPullFlag : private VirtEnumStorage<virDomainBlockPullFlags>,
                              public VirtEnumBase<BlockPullFlag>,
                              public EnumSetHelper<BlockPullFlag> {
    friend VirtEnumBase<BlockPullFlag>;
    friend EnumSetHelper<BlockPullFlag>;
    enum class Underlying {
        BANDWIDTH_BYTES = VIR_DOMAIN_BLOCK_PULL_BANDWIDTH_BYTES, /* bandwidth in bytes/s instead of MiB/s */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"", "", "", "", "", "", "bandwidth_bytes"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto BANDWIDTH_BYTES = Underlying::BANDWIDTH_BYTES;
};

}