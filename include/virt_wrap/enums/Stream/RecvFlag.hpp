#pragma once
#include <libvirt/libvirt.h>
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"

namespace virt {

class Stream::RecvFlag : private VirtEnumStorage<virStreamRecvFlagsValues>, public VirtEnumBase<RecvFlag>, public EnumSetHelper<RecvFlag> {
    friend VirtEnumBase<RecvFlag>;
    friend EnumSetHelper<RecvFlag>;
    enum class Underlying {
        STOP_AT_HOLE = VIR_STREAM_RECV_STOP_AT_HOLE,
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"stop_at_hole"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto STOP_AT_HOLE = Underlying::STOP_AT_HOLE;
};

}