#pragma once
#include "virt_wrap/enums/GFlags.hpp"
#include "ModificationImpactFlag.hpp"

namespace virt::enums::domain {
using MITPFlags = EnumSetTie<ModificationImpactFlag, TypedParameterFlag>;
}