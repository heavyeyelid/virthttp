#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::GetJobStatsFlag : private VirtEnumStorage<virDomainGetJobStatsFlags>,
                                public VirtEnumBase<GetJobStatsFlag>,
                                public EnumSetHelper<GetJobStatsFlag> {
    friend VirtEnumBase<GetJobStatsFlag>;
    friend EnumSetHelper<GetJobStatsFlag>;
    enum class Underlying {
        COMPLETED = VIR_DOMAIN_JOB_STATS_COMPLETED, /* return stats of a recently completed job */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"completed"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto COMPLETED = Underlying::COMPLETED;
};

}