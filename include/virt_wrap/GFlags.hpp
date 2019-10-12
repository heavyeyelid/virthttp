#pragma once

#include <array>
#include <libvirt/libvirt.h>
#include "enums/Base.hpp"
#include "utility.hpp"

namespace virt {

class TypedParameterFlag : private VirtEnumStorage<virTypedParameterFlags>,
                           public VirtEnumBase<TypedParameterFlag>,
                           EnumSetHelper<TypedParameterFlag> {
    friend VirtEnumBase<TypedParameterFlag>;
    friend EnumSetHelper<TypedParameterFlag>;
    enum class Underlying { STRING_OKAY = VIR_TYPED_PARAM_STRING_OKAY };

  protected:
    constexpr static std::array values = {"string_okay"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto STRING_OKAY = Underlying::STRING_OKAY;
};

}