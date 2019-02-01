//
// Created by _as on 2019-02-01.
//

#pragma once

#include <libvirt/libvirt.h>
#include <lift.hpp>
#include "../TypesParam.hpp"
#include "../utility.hpp"

namespace virt {
  inline TypedParams::~TypedParams() noexcept {
    if(underlying)
      virTypedParamsFree(underlying, size);
  }

  void TypedParams::add(gsl::czstring<> name, int i) {
    virTypedParamsAddInt(&underlying, &size, &capacity, name, i);
  }
  void TypedParams::add(gsl::czstring<> name, unsigned u){
    virTypedParamsAddUInt(&underlying, &size, &capacity, name, u);
  }
  void TypedParams::add(gsl::czstring<> name, long long ll){
    virTypedParamsAddLLong(&underlying, &size, &capacity, name, ll);
  }
  void TypedParams::add(gsl::czstring<> name, unsigned long long ull){
    virTypedParamsAddULLong(&underlying, &size, &capacity, name, ull);
  }
  void TypedParams::add(gsl::czstring<> name, double d){
    virTypedParamsAddDouble(&underlying, &size, &capacity, name, d);
  }
  void TypedParams::add(gsl::czstring<> name, bool b) {
    virTypedParamsAddBoolean(&underlying, &size, &capacity, name, b);
  }
  void TypedParams::add(gsl::czstring<> name, gsl::czstring<> czs){
    virTypedParamsAddString(&underlying, &size, &capacity, name, czs);
  }
  void TypedParams::add(const TypedParameter& tp){
    const auto name = tp.name.data();
    std::visit(Visitor{ // I hate when I can't just lift a member overload set //C++2aTODO change captures to [=, this]
      [&, this](int v){add(name, v);},
      [&, this](unsigned v){add(name, v);},
      [&, this](long long v){add(name, v);},
      [&, this](unsigned long long v){add(name, v);},
      [&, this](double v){add(name, v);},
      [&, this](bool v){add(name, v);},
      [&, this](gsl::czstring<> v){add(name, v);}
    }, tp.val);
  }
}