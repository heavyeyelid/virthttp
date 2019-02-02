//
// Created by _as on 2019-01-31.
//

#pragma once

#include <stdexcept>
#include <vector>
#include <gsl/gsl>
#include <libvirt/libvirt-domain.h>
#include "fwd.hpp"

namespace virt {
  class Domain {
    friend Connection;

    virDomainPtr underlying = nullptr;
    inline explicit Domain(virDomainPtr ptr) noexcept;

  public:
    using Info = virDomainInfo;
    struct Stats {
      enum class Types {
        STATE = VIR_DOMAIN_STATS_STATE, // domain state
        CPU_TOTAL	=	VIR_DOMAIN_STATS_CPU_TOTAL, // domain CPU info
        BALLOON = VIR_DOMAIN_STATS_BALLOON, // domain balloon info
        VCPU	=	VIR_DOMAIN_STATS_VCPU, // domain virtual CPU info
        INTERFACE = VIR_DOMAIN_STATS_INTERFACE, // domain interfaces info
        BLOCK	=	VIR_DOMAIN_STATS_BLOCK, // domain block info
        PERF = VIR_DOMAIN_STATS_PERF, // domain perf event info
        //IOTHREAD	=	VIR_DOMAIN_STATS_IOTHREAD, // iothread poll info
      };
      class Record;
    };
    enum class ShutdownFlags {
      DEFAULT        = VIR_DOMAIN_REBOOT_DEFAULT,        /* hypervisor choice */
      ACPI_POWER_BTN = VIR_DOMAIN_REBOOT_ACPI_POWER_BTN, /* Send ACPI event */
      GUEST_AGENT    = VIR_DOMAIN_REBOOT_GUEST_AGENT,    /* Use guest agent */
      INITCTL        = VIR_DOMAIN_REBOOT_INITCTL,        /* Use initctl */
      SIGNAL         = VIR_DOMAIN_REBOOT_SIGNAL,         /* Send a signal */
      PARAVIRT       = VIR_DOMAIN_REBOOT_PARAVIRT,       /* Use paravirt guest control */
    };
    enum class CreateFlags {
      NONE         = VIR_DOMAIN_NONE,
      PAUSED       = VIR_DOMAIN_START_PAUSED,
      AUTODESTROY  = VIR_DOMAIN_START_AUTODESTROY,
      BYPASS_CACHE = VIR_DOMAIN_START_BYPASS_CACHE,
      FORCE_BOOT   = VIR_DOMAIN_START_FORCE_BOOT,
      VALIDATE     = VIR_DOMAIN_START_VALIDATE
    };
    enum class State {
      NOSTATE     = VIR_DOMAIN_NOSTATE,     /* no state */
      RUNNING     = VIR_DOMAIN_RUNNING,     /* the domain is running */
      BLOCKED     = VIR_DOMAIN_BLOCKED,     /* the domain is blocked on resource */
      PAUSED      = VIR_DOMAIN_PAUSED,      /* the domain is paused by user */
      SHUTDOWN    = VIR_DOMAIN_SHUTDOWN,    /* the domain is being shut down */
      SHUTOFF     = VIR_DOMAIN_SHUTOFF,     /* the domain is shut off */
      CRASHED     = VIR_DOMAIN_CRASHED,     /* the domain is crashed */
      PMSUSPENDED = VIR_DOMAIN_PMSUSPENDED, /* the domain is suspended by guest power management */
    };
    enum class UndefineFlags {
      MANAGED_SAVE       = VIR_DOMAIN_UNDEFINE_MANAGED_SAVE,       /* Also remove any managed save */
      SNAPSHOTS_METADATA = VIR_DOMAIN_UNDEFINE_SNAPSHOTS_METADATA, /* If last use of domain, then also remove any snapshot metadata */
      NVRAM              = VIR_DOMAIN_UNDEFINE_NVRAM,              /* Also remove any nvram file */
      KEEP_NVRAM         = VIR_DOMAIN_UNDEFINE_KEEP_NVRAM,         /* Keep nvram file */
    };

    Domain(const Domain&) = delete;
    Domain(Domain&&) noexcept = default;
    inline ~Domain() noexcept;

    void create();
    Info getInfo() const;
    unsigned char getUUID() const;
    bool isActive() const;

    void reboot(ShutdownFlags flags = ShutdownFlags::DEFAULT);
    void rename(gsl::czstring<>);
    void reset();
    void resume();
    void setAutoStart(bool);
    void shutdown(ShutdownFlags flags = ShutdownFlags::DEFAULT);
    void suspend();
    void destroy();
    void undefine(UndefineFlags = UndefineFlags(0));

    static Domain createXML(Connection&, gsl::czstring<> xml, CreateFlags flags = CreateFlags::NONE);
    static Domain defineXML();
  };

  class Domain::Stats::Record {
    friend Connection;

    Domain dom;
    std::vector<TypedParameter> params{};

    explicit Record(const virDomainStatsRecord&) noexcept;

  public:
  };

  constexpr inline Domain::Stats::Types operator|(Domain::Stats::Types lhs, Domain::Stats::Types rhs) noexcept;
}
