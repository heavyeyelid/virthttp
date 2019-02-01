//
// Created by _as on 2019-02-01.
//

#pragma once

#include "../type_ops.hpp"
#include "../Domain.hpp"

namespace virt {
  inline Domain::Domain(virDomainPtr ptr) noexcept : underlying(ptr) {}

  inline Domain::~Domain() noexcept {
    virDomainFree(underlying);
  }

  void Domain::create(){
    if(virDomainCreate(underlying))
      throw std::runtime_error{"virDomainCreate"};
  }

  constexpr inline Domain::Stats::Types operator|(Domain::Stats::Types lhs, Domain::Stats::Types rhs) noexcept {
    return Domain::Stats::Types(to_integral(lhs) | to_integral(rhs));
  }
}