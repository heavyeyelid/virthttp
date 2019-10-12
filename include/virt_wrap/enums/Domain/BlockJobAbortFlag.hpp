#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::BlockJobAbortFlag : private VirtEnumStorage<virDomainBlockJobAbortFlags>,
                                  public VirtEnumBase<BlockJobAbortFlag>,
                                  public EnumSetHelper<BlockJobAbortFlag> {
    friend VirtEnumBase<BlockJobAbortFlag>;
    friend EnumSetHelper<BlockJobAbortFlag>;
    enum class Underlying {
        ASYNC = VIR_DOMAIN_BLOCK_JOB_ABORT_ASYNC,
        PIVOT = VIR_DOMAIN_BLOCK_JOB_ABORT_PIVOT,
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"async", "pivot"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto ASYNC = Underlying::ASYNC;
    constexpr static auto PIVOT = Underlying::PIVOT;
};

}