#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::BlockJobSetSpeedFlag : private VirtEnumStorage<virDomainBlockJobSetSpeedFlags>,
                                     public VirtEnumBase<BlockJobSetSpeedFlag>,
                                     public EnumSetHelper<BlockJobSetSpeedFlag> {
    friend VirtEnumBase<BlockJobSetSpeedFlag>;
    friend EnumSetHelper<BlockJobSetSpeedFlag>;
    enum class Underlying {
        BANDWIDTH_BYTES = VIR_DOMAIN_BLOCK_JOB_SPEED_BANDWIDTH_BYTES, /* bandwidth in bytes/s instead of MiB/s */
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