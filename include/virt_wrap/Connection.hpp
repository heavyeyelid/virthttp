#pragma once

#include <cstring>
#include <exception>
#include <gsl/gsl>
#include <libvirt/libvirt.h>
#include "fwd.hpp"
#include "Domain.hpp"
#include "type_ops.hpp"
#include "utility.hpp"

namespace virt {

  unsigned long getVersion();

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

      explicit ConnectCredential(const virConnectCredential& in) noexcept;
  };

  using ConnectAuthCallback = bool (*)(gsl::span<ConnectCredential>);

  template <typename Callback = ConnectAuthCallback>
  class ConnectionAuth {
  public:

    template <typename Container>
    inline ConnectionAuth(Container c, Callback callback);

    inline ConnectionAuth(const ConnectionAuth&) = default;
    inline ConnectionAuth(ConnectionAuth&&) noexcept = default;


    explicit operator virConnectAuth() noexcept;


    std::vector<CredentialType> cred_types{};
    Callback callback;
  };

  class Connection {
    friend Domain;

    virConnectPtr underlying = nullptr;

  public:
    enum class Flags : unsigned {
      RO         = VIR_CONNECT_RO,  /* A readonly connection */
      NO_ALIASES = VIR_CONNECT_NO_ALIASES,  /* Don't try to resolve URI aliases */
    };
    struct List {
      struct Domains {
        enum class Flags : unsigned {
          DEFAULT = 0,
          ACTIVE         = VIR_CONNECT_LIST_DOMAINS_ACTIVE,
          INACTIVE       = VIR_CONNECT_LIST_DOMAINS_INACTIVE,

          PERSISTENT     = VIR_CONNECT_LIST_DOMAINS_PERSISTENT,
          TRANSIENT      = VIR_CONNECT_LIST_DOMAINS_TRANSIENT,

          RUNNING        = VIR_CONNECT_LIST_DOMAINS_RUNNING,
          PAUSED         = VIR_CONNECT_LIST_DOMAINS_PAUSED,
          SHUTOFF        = VIR_CONNECT_LIST_DOMAINS_SHUTOFF,
          OTHER          = VIR_CONNECT_LIST_DOMAINS_OTHER,

          MANAGEDSAVE    = VIR_CONNECT_LIST_DOMAINS_MANAGEDSAVE,
          NO_MANAGEDSAVE = VIR_CONNECT_LIST_DOMAINS_NO_MANAGEDSAVE,

          AUTOSTART = VIR_CONNECT_LIST_DOMAINS_AUTOSTART,
          NO_AUTOSTART = VIR_CONNECT_LIST_DOMAINS_NO_AUTOSTART,

          HAS_SNAPSHOT = VIR_CONNECT_LIST_DOMAINS_HAS_SNAPSHOT,
          NO_SNAPSHOT = VIR_CONNECT_LIST_DOMAINS_NO_SNAPSHOT,
        };
      };
    };
    struct GetAllDomains {
      struct Stats {
        enum class Flags : unsigned {
          ACTIVE	=	VIR_CONNECT_GET_ALL_DOMAINS_STATS_ACTIVE,
          INACTIVE	=	VIR_CONNECT_GET_ALL_DOMAINS_STATS_INACTIVE,
          OTHER	=	VIR_CONNECT_GET_ALL_DOMAINS_STATS_OTHER,
          PAUSED	=	VIR_CONNECT_GET_ALL_DOMAINS_STATS_PAUSED,
          PERSISTENT	=	VIR_CONNECT_LIST_DOMAINS_PERSISTENT,
          RUNNING	=	VIR_CONNECT_LIST_DOMAINS_RUNNING,
          SHUTOFF	=	VIR_CONNECT_LIST_DOMAINS_SHUTOFF,
          TRANSIENT	=	VIR_CONNECT_LIST_DOMAINS_TRANSIENT,
          NOWAIT	=	VIR_CONNECT_GET_ALL_DOMAINS_STATS_NOWAIT, // report statistics that can be obtained immediately without any blocking
          BACKING	=	VIR_CONNECT_GET_ALL_DOMAINS_STATS_BACKING, // include backing chain for block stats
          ENFORCE_STATS	= VIR_CONNECT_GET_ALL_DOMAINS_STATS_ENFORCE_STATS, // enforce requested stats
        };
      };
    };


    inline Connection(gsl::czstring<> name, bool rd_only = false) noexcept;

    template <typename Callback = virConnectAuthCallbackPtr>
    inline Connection(gsl::czstring<> name, ConnectionAuth<Callback>& auth, Flags flags) noexcept;
    inline Connection(const Connection& conn) noexcept = delete;
    constexpr Connection(Connection&& conn) noexcept = default;
    inline Connection& operator=(const Connection& conn) noexcept = delete;
    inline Connection& operator=(Connection&& conn) noexcept = default;

    inline ~Connection();

    void ref();

    template <typename Data>
    void registerCloseCallback(void(*cb)(Data&), std::unique_ptr<Data> data = nullptr);
    void registerCloseCallback(void(*cb)());

    template <typename Data>
    void unregisterCloseCallback(void(*cb)(Data&));
    void unregisterCloseCallback(void(*cb)());

    void setKeepAlive(int interval, unsigned count);

    inline gsl::zstring<> getCapabilities() const noexcept;
    inline gsl::zstring<> getHostname() const noexcept;

    unsigned long	getLibVersion() const;

    int getMaxVcpus(gsl::czstring<> type) const noexcept;

    gsl::zstring<> getSysInfo(unsigned flags) const noexcept;

    gsl::czstring<> getType() const noexcept;
    gsl::zstring<> getURI() const noexcept;

    inline bool isAlive() const noexcept;
    inline bool isEncrypted() const noexcept;
    inline bool isSecure() const noexcept;

    inline int numOfDomains() const noexcept;
    inline int numOfDefinedDomains() const noexcept;

    auto listDomains() const -> std::vector<int>;
    template <typename StrT>
    auto listDefinedDomains() const = delete;


    auto listAllDomains(List::Domains::Flags flags = List::Domains::Flags::DEFAULT) const -> std::vector<Domain>;

    auto getAllDomainStats(Domain::Stats::Types stats, Connection::GetAllDomains::Stats::Flags flags) -> std::vector<Domain::Stats::Record>;

    Domain domainLookupByID(int) const noexcept;
    Domain domainLookupByName(gsl::czstring<>) const noexcept;
    Domain domainLookupByUUIDString(gsl::czstring<>) const noexcept;

    virNodeInfo nodeGetInfo() const;
    auto nodeGetFreeMemory() const;
    auto nodeGetCellsFreeMemory() const;

    constexpr explicit operator bool() const noexcept { return underlying != nullptr; }
  };


  constexpr inline Connection::Flags operator|(Connection::Flags lhs, Connection::Flags rhs) noexcept;
  constexpr inline Connection::List::Domains::Flags operator|(Connection::List::Domains::Flags lhs, Connection::List::Domains::Flags rhs) noexcept;
  constexpr inline Connection::GetAllDomains::Stats::Flags operator|(Connection::GetAllDomains::Stats::Flags lhs, Connection::GetAllDomains::Stats::Flags rhs) noexcept;
}