#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::BlockResizeFlag : private VirtEnumStorage<virDomainBlockResizeFlags>,
                                public VirtEnumBase<BlockResizeFlag>,
                                public EnumSetHelper<BlockResizeFlag> {
    friend VirtEnumBase<BlockResizeFlag>;
    friend EnumSetHelper<BlockResizeFlag>;
    enum class Underlying {
        RESIZE_BYTES = VIR_DOMAIN_BLOCK_RESIZE_BYTES, /* size in bytes instead of KiB */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"bytes"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto RESIZE_BYTES = Underlying::RESIZE_BYTES;
};

}