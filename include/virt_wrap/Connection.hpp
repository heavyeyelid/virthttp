#pragma once

#include <cstring>
#include <exception>
#include <gsl/gsl>
#include <libvirt/libvirt.h>
#include "type_ops.hpp"

namespace virt {

  unsigned long getVersion(){
    unsigned long ret{};
    if(virGetVersion(&ret, nullptr, nullptr))
      throw std::runtime_error{"virGetVersion"};
    return ret;
  }

  enum class CredentialType : int {
    AUTHNAME = VIR_CRED_AUTHNAME, // Identify to authorize as
    CNONCE = VIR_CRED_CNONCE, // Client supplies a nonce
    ECHOPROMPT = VIR_CRED_ECHOPROMPT, // Challenge response
    EXTERNAL = VIR_CRED_EXTERNAL, // Externally managed credential More may be added - expect the unexpected
    LANGUAGE = VIR_CRED_LANGUAGE, //RFC 1766 languages, comma separated
    NOECHOPROMPT = VIR_CRED_NOECHOPROMPT, // Challenge response
    PASSPHRASE = VIR_CRED_PASSPHRASE, // Passphrase secret
    REALM = VIR_CRED_REALM, // Authentication realm
    USERNAME = VIR_CRED_USERNAME, // Identity to act as
  };


  struct ConnectCredential {
      const CredentialType type;
      const gsl::czstring<> prompt; /* Prompt to show to user */
      const gsl::czstring<> challenge; /* Additional challenge to show */
      const gsl::czstring<>  defresult; /* Optional default result */
      std::string result{};

      ConnectCredential(const virConnectCredential& in) :
          type(CredentialType(in.type)),
          prompt(in.prompt),
          challenge(in.challenge),
          defresult(in.defresult) {
      }
  };

  using ConnectAuthCallback = bool (*)(gsl::span<ConnectCredential>);

  template <typename Callback = ConnectAuthCallback>
  class ConnectionAuth {
  public:

    template <typename Container>
    inline ConnectionAuth(Container c, Callback callback) : callback(callback) {
      if constexpr (std::is_same_v<Container, std::vector<CredentialType>>)
          cred_types = std::forward(c);
      else {
        cred_types.reserve(c.size());
        std::copy(c.begin(), c.end(), std::back_inserter(cred_types));
      }

    }

    inline ConnectionAuth(const ConnectionAuth&) = default;
    inline ConnectionAuth(ConnectionAuth&&) noexcept = default;


    explicit operator virConnectAuth() noexcept {
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


    std::vector<CredentialType> cred_types{};
    Callback callback;
  };

  class Connection {
    virConnectPtr underlying = nullptr;

  public:
    enum class Flags : unsigned {
      RO         = (1 << 0),  /* A readonly connection */
      NO_ALIASES = (1 << 1),  /* Don't try to resolve URI aliases */
    };


    inline Connection(gsl::czstring<> name, bool rd_only = false) noexcept {
      underlying = rd_only ? virConnectOpenReadOnly(name) : virConnectOpen(name);
    }

    template <typename Callback = virConnectAuthCallbackPtr>
    inline Connection(gsl::czstring<> name, ConnectionAuth<Callback>& auth, Flags flags) noexcept {
      virConnectAuth c_auth = auth;
      underlying = virConnectOpenAuth(name, &c_auth, to_integral(flags));
    }

    inline Connection(Connection&& conn) noexcept {
      underlying = conn.underlying;
    }

    inline ~Connection(){
      if(underlying)
        virConnectClose(underlying);
    }

    void ref(){
      if(virConnectRef(underlying))
        throw std::runtime_error{"virConnectRef"};
    }

    template <typename Data>
    void registerCloseCallback(void(*cb)(Data&), std::unique_ptr<Data> data = nullptr){
      virConnectRegisterCloseCallback(underlying, reinterpret_cast<virConnectCloseFunc>(cb), &data, &std::unique_ptr<Data>::get_deleter());
    }
    template <typename Data>
    void registerCloseCallback(void(*cb)()){
      if(virConnectRegisterCloseCallback(underlying, reinterpret_cast<virConnectCloseFunc>(cb), nullptr, nullptr))
        throw std::runtime_error{"virConnectRegisterCloseCallback"};
    }

    template <typename Data>
    void unregisterCloseCallback(void(*cb)(Data&)){
      if(virConnectUnregisterCloseCallback(underlying, reinterpret_cast<virConnectCloseFunc>(cb)))
        throw std::runtime_error{"unregisterCloseCallback"};
    }

    void unregisterCloseCallback(void(*cb)()){
      if(virConnectUnregisterCloseCallback(underlying, reinterpret_cast<virConnectCloseFunc>(cb)))
        throw std::runtime_error{"unregisterCloseCallback"};
    }

    void setKeepAlive(int interval, unsigned count) {
      if (virConnectSetKeepAlive(underlying, interval, count))
        throw std::runtime_error{"virConnectSetKeepAlive"};
    }

    inline gsl::zstring<> getCapabilities() const noexcept {
      return virConnectGetCapabilities(underlying);
    }
    inline gsl::zstring<> getHostname() const noexcept {
      return virConnectGetHostname(underlying);
    }

    unsigned long	getLibVersion() const {
      unsigned long ret{};
      if(virConnectGetLibVersion(underlying, &ret))
        throw std::runtime_error{"virConnectGetLibVersion"};
      return ret;
    }

    int getMaxVcpus(gsl::czstring<> type) const noexcept {
      return virConnectGetMaxVcpus(underlying, type);
    }

    gsl::zstring<> getSysInfo(unsigned flags) const noexcept {
      return virConnectGetSysinfo(underlying, flags);
    }

    gsl::czstring<> getType() const noexcept {
      return virConnectGetType(underlying);
    }
    gsl::zstring<> getURI() const noexcept {
      return virConnectGetURI(underlying);
    }

    inline bool isAlive() const noexcept {
      return virConnectIsAlive(underlying) > 0;
    }
    inline bool isEncrypted() const noexcept {
      return virConnectIsEncrypted(underlying) > 0;
    }
    inline bool isSecure() const noexcept {
      return virConnectIsSecure(underlying) > 0;
    }

  };



  Connection::Flags operator|(Connection::Flags lhs, Connection::Flags rhs){
    return Connection::Flags(to_integral(lhs) | to_integral(rhs));
  }
}