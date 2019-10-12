#pragma once
#include <libvirt/libvirt.h>
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"

namespace virt {

class Stream::Flag : private VirtEnumStorage<virStreamFlags>, public VirtEnumBase<Flag>, public EnumSetHelper<Flag> {
    friend VirtEnumBase<Flag>;
    friend EnumSetHelper<Flag>;
    enum class Underlying {
        NONBLOCK = VIR_STREAM_NONBLOCK,
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"nonblock"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto NONBLOCK = Underlying::NONBLOCK;
};

}