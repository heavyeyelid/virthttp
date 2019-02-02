//
// Created by _as on 2019-02-01.
//

#pragma once

#include <algorithm>
#include <libvirt/libvirt.h>
#include "../type_ops.hpp"
#include "../Domain.hpp"
#include "../TypesParam.hpp"

namespace virt {
  inline Domain::Domain(virDomainPtr ptr) noexcept : underlying(ptr) {}

  inline Domain::~Domain() noexcept {
    virDomainFree(underlying);
  }

  void Domain::create(){
    if(virDomainCreate(underlying))
      throw std::runtime_error{"virDomainCreate"};
  }

  Domain::Stats::Record::Record(const virDomainStatsRecord& from) noexcept : dom(from.dom) {
    params.reserve(static_cast<std::size_t>(from.nparams));
    std::transform(from.params, from.params + from.nparams, std::back_inserter(params), [](const virTypedParameter& tp){return TypedParameter{tp};});
  }

  constexpr inline Domain::Stats::Types operator|(Domain::Stats::Types lhs, Domain::Stats::Types rhs) noexcept {
    return Domain::Stats::Types(to_integral(lhs) | to_integral(rhs));
  }
}