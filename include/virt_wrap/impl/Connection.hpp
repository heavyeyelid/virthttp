//
// Created by _as on 2019-02-01.
//

#pragma once

#include <cstring>
#include <exception>
#include <vector>
#include <gsl/gsl>
#include <libvirt/libvirt.h>
#include "../fwd.hpp"
#include "../type_ops.hpp"
#include "../utility.hpp"
#include "../Connection.hpp"
#include "../Domain.hpp"

namespace virt {

  unsigned long getVersion(){
    unsigned long ret{};
    if(virGetVersion(&ret, nullptr, nullptr))
      throw std::runtime_error{"virGetVersion"};
    return ret;
  }


  ConnectCredential::ConnectCredential(const virConnectCredential& in) noexcept :
      type(CredentialType(in.type)),
      prompt(in.prompt),
      challenge(in.challenge),
      defresult(in.defresult) {}



    template <typename Callback>
    template <typename Container>
    inline ConnectionAuth<Callback>::ConnectionAuth(Container c, Callback callback) : callback(callback) {
      if constexpr (std::is_same_v<Container, std::vector<CredentialType>>)
        cred_types = std::forward(c);
      else {
        cred_types.reserve(c.size());
        std::copy(c.begin(), c.end(), std::back_inserter(cred_types));
      }

    }

  template <typename Callback>
  ConnectionAuth<Callback>::operator virConnectAuth() noexcept {
    virConnectAuth ret;
    ret.credtype = reinterpret_cast<int*>(cred_types.data());
    ret.ncredtype = gsl::narrow_cast<unsigned>(cred_types.size());
    ret.cbdata = reinterpret_cast<void*>(&callback);

    ret.cb = +[](virConnectCredentialPtr creds_ptr, unsigned int ncreds, void* cbdata) -> int {
      std::vector<ConnectCredential> creds{};
      creds.reserve(ncreds);
      std::transform(creds_ptr, creds_ptr + ncreds, std::back_inserter(creds), [](const auto& cred){return ConnectCredential{cred};});
      const bool ret = (*reinterpret_cast<Callback*>(cbdata))(creds);
      auto it = creds_ptr;
      for (const auto& [t, p, c, d, res] : creds){
        it->result = static_cast<char*>(malloc(it->resultlen = res.length() + 1u));
        std::copy(res.begin(), res.end(), it->result);
        ++it;
      }
      return ret;
    };

    return ret;
  }



  inline Connection::Connection(gsl::czstring<> name, bool rd_only) noexcept {
    underlying = rd_only ? virConnectOpenReadOnly(name) : virConnectOpen(name);
  }

  template <typename Callback>
  inline Connection::Connection(gsl::czstring<> name, ConnectionAuth<Callback>& auth, Flags flags) noexcept {
    virConnectAuth c_auth = auth;
    underlying = virConnectOpenAuth(name, &c_auth, to_integral(flags));
  }

  inline Connection::Connection(Connection&& conn) noexcept {
    underlying = conn.underlying;
  }

  inline Connection::~Connection(){
    if(underlying)
      virConnectClose(underlying);
  }

  void Connection::ref(){
    if(virConnectRef(underlying))
      throw std::runtime_error{"virConnectRef"};
  }

  template <typename Data>
  void Connection::registerCloseCallback(void(*cb)(Data&), std::unique_ptr<Data> data){
    virConnectRegisterCloseCallback(underlying, reinterpret_cast<virConnectCloseFunc>(cb), &data, &std::unique_ptr<Data>::get_deleter());
  }
  void Connection::registerCloseCallback(void(*cb)()){
    if(virConnectRegisterCloseCallback(underlying, reinterpret_cast<virConnectCloseFunc>(cb), nullptr, nullptr))
      throw std::runtime_error{"virConnectRegisterCloseCallback"};
  }

  template <typename Data>
  void Connection::unregisterCloseCallback(void(*cb)(Data&)){
    if(virConnectUnregisterCloseCallback(underlying, reinterpret_cast<virConnectCloseFunc>(cb)))
      throw std::runtime_error{"unregisterCloseCallback"};
  }

  void Connection::unregisterCloseCallback(void(*cb)()){
    if(virConnectUnregisterCloseCallback(underlying, reinterpret_cast<virConnectCloseFunc>(cb)))
      throw std::runtime_error{"unregisterCloseCallback"};
  }

  void Connection::setKeepAlive(int interval, unsigned count) {
    if (virConnectSetKeepAlive(underlying, interval, count))
      throw std::runtime_error{"virConnectSetKeepAlive"};
  }

  inline gsl::zstring<> Connection::getCapabilities() const noexcept {
    return virConnectGetCapabilities(underlying);
  }
  inline gsl::zstring<> Connection::getHostname() const noexcept {
    return virConnectGetHostname(underlying);
  }

  unsigned long	Connection::getLibVersion() const {
    unsigned long ret{};
    if(virConnectGetLibVersion(underlying, &ret))
      throw std::runtime_error{"virConnectGetLibVersion"};
    return ret;
  }

  int Connection::getMaxVcpus(gsl::czstring<> type) const noexcept {
    return virConnectGetMaxVcpus(underlying, type);
  }

  gsl::zstring<> Connection::getSysInfo(unsigned flags) const noexcept {
    return virConnectGetSysinfo(underlying, flags);
  }

  gsl::czstring<> Connection::getType() const noexcept {
    return virConnectGetType(underlying);
  }
  gsl::zstring<> Connection::getURI() const noexcept {
    return virConnectGetURI(underlying);
  }

  inline bool Connection::isAlive() const noexcept {
    return virConnectIsAlive(underlying) > 0;
  }
  inline bool Connection::isEncrypted() const noexcept {
    return virConnectIsEncrypted(underlying) > 0;
  }
  inline bool Connection::isSecure() const noexcept {
    return virConnectIsSecure(underlying) > 0;
  }

  inline int Connection::numOfDomains() const noexcept {
    return virConnectNumOfDomains(underlying);
  }

  std::vector<int> Connection::listDomains() const {
    std::vector<int> ret{};
    const auto max_size = numOfDomains();
    ret.resize(gsl::narrow_cast<unsigned>(max_size));
    const auto res = virConnectListDomains(underlying, ret.data(), max_size);
    if(res < 0)
      throw std::runtime_error{"virConnectListDomains"};
    ret.resize(gsl::narrow_cast<unsigned>(res));
    return ret;
  }

  std::vector<Domain> Connection::listAllDomains(List::Domains::Flags flags) const {
    std::vector<Domain> ret;
    virDomainPtr *domains;

    const auto res = virConnectListAllDomains(underlying, &domains, to_integral(flags));
    if(res < 0)
      throw std::runtime_error{"virConnectListAllDomains"};

    ret.reserve(res);
    std::transform(domains, domains + res, std::back_inserter(ret), [](virDomainPtr d){return Domain{d};});
    std::for_each(domains, domains + res, virDomainFree);
    free(domains);

    return ret;
  }

  auto Connection::getAllDomainStats(Domain::Stats::Types stats, Connection::GetAllDomains::Stats::Flags flags){
    virDomainStatsRecordPtr* ptr;
    virConnectGetAllDomainStats(underlying, to_integral(stats), &ptr, to_integral(flags));
  }


  constexpr inline Connection::Flags operator|(Connection::Flags lhs, Connection::Flags rhs) noexcept {
    return Connection::Flags(to_integral(lhs) | to_integral(rhs));
  }
  constexpr inline Connection::List::Domains::Flags operator|(Connection::List::Domains::Flags lhs, Connection::List::Domains::Flags rhs) noexcept {
    return Connection::List::Domains::Flags(to_integral(lhs) | to_integral(rhs));
  }
  constexpr inline Connection::GetAllDomains::Stats::Flags operator|(Connection::GetAllDomains::Stats::Flags lhs, Connection::GetAllDomains::Stats::Flags rhs) noexcept {
    return Connection::GetAllDomains::Stats::Flags(to_integral(lhs) | to_integral(rhs));
  }
}