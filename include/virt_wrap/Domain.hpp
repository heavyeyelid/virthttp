//
// Created by _as on 2019-01-31.
//

#pragma once

#include <libvirt/libvirt-domain.h>
#include <stdexcept>
#include <vector>
#include "fwd.hpp"

namespace virt {
  class Domain {
    friend Connection;

    virDomainPtr underlying = nullptr;
    inline explicit Domain(virDomainPtr ptr) noexcept;

  public:
    struct Stats {
      enum class Types {
        STATE = VIR_DOMAIN_STATS_STATE, // domain state
        CPU_TOTAL	=	VIR_DOMAIN_STATS_CPU_TOTAL, // domain CPU info
        BALLOON = VIR_DOMAIN_STATS_BALLOON, // domain balloon info
        VCPU	=	VIR_DOMAIN_STATS_VCPU, // domain virtual CPU info
        INTERFACE = VIR_DOMAIN_STATS_INTERFACE, // domain interfaces info
        BLOCK	=	VIR_DOMAIN_STATS_BLOCK, // domain block info
        PERF = VIR_DOMAIN_STATS_PERF, // domain perf event info
        IOTHREAD	=	VIR_DOMAIN_STATS_IOTHREAD, // iothread poll info
      };
      class Record;
    };

    inline ~Domain() noexcept;

    void create();

    static Domain createXML();
    static Domain defineXML();
  };

  class Domain::Stats::Record {
    Domain dom;
    std::vector<TypedParameter> params;
  };

  constexpr inline Domain::Stats::Types operator|(Domain::Stats::Types lhs, Domain::Stats::Types rhs) noexcept;
}
