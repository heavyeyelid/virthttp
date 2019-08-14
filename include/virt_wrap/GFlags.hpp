#pragma once

#include <array>
#include <libvirt/libvirt.h>
#include "utility.hpp"

namespace virt {
enum class TypedParameterFlag { STRING_OKAY = VIR_TYPED_PARAM_STRING_OKAY };
class TypedParameterFlagC : EnumSetHelper<TypedParameterFlagC, TypedParameterFlag> {
    using Base = EnumSetHelper<TypedParameterFlagC, TypedParameterFlag>;
    friend Base;
    constexpr static std::array values = {"string_okay"};
};
}