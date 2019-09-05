#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::MigrateFlag : private VirtEnumStorage<virDomainMigrateFlags>, public VirtEnumBase<MigrateFlag>, public EnumSetHelper<MigrateFlag> {
    friend VirtEnumBase<MigrateFlag>;
    friend EnumSetHelper<MigrateFlag>;
    enum class Underlying {
        LIVE = VIR_MIGRATE_LIVE,
        PEER2PEER = VIR_MIGRATE_PEER2PEER,
        TUNNELLED = VIR_MIGRATE_TUNNELLED,
        PERSIST_DEST = VIR_MIGRATE_PERSIST_DEST,
        UNDEFINE_SOURCE = VIR_MIGRATE_UNDEFINE_SOURCE,
        PAUSED = VIR_MIGRATE_PAUSED,
        NON_SHARED_DISK = VIR_MIGRATE_NON_SHARED_DISK,
        NON_SHARED_INC = VIR_MIGRATE_NON_SHARED_INC,
        CHANGE_PROTECTION = VIR_MIGRATE_CHANGE_PROTECTION,
        UNSAFE = VIR_MIGRATE_UNSAFE,
        OFFLINE = VIR_MIGRATE_OFFLINE,
        COMPRESSED = VIR_MIGRATE_COMPRESSED,
        ABORT_ON_ERROR = VIR_MIGRATE_ABORT_ON_ERROR,
        AUTO_CONVERGE = VIR_MIGRATE_AUTO_CONVERGE,
        RDMA_PIN_ALL = VIR_MIGRATE_RDMA_PIN_ALL,
        POSTCOPY = VIR_MIGRATE_POSTCOPY,
        TLS = VIR_MIGRATE_TLS,
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {
        "live",           "peer2peer",         "tunnelled", "persist_dest", "undefine_source", "paused",         "non_shared_disk",
        "non_shared_inc", "change_protection", "unsafe",    "offline",      "compressed",      "abort_on_error", "auto_converge",
        "rdma_pin_all",   "postcopy",          "tls"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto LIVE = Underlying::LIVE;
    constexpr static auto PEER2PEER = Underlying::PEER2PEER;
    constexpr static auto TUNNELLED = Underlying::TUNNELLED;
    constexpr static auto PERSIST_DEST = Underlying::PERSIST_DEST;
    constexpr static auto UNDEFINE_SOURCE = Underlying::UNDEFINE_SOURCE;
    constexpr static auto PAUSED = Underlying::PAUSED;
    constexpr static auto NON_SHARED_DISK = Underlying::NON_SHARED_DISK;
    constexpr static auto NON_SHARED_INC = Underlying::NON_SHARED_INC;
    constexpr static auto CHANGE_PROTECTION = Underlying::CHANGE_PROTECTION;
    constexpr static auto UNSAFE = Underlying::UNSAFE;
    constexpr static auto OFFLINE = Underlying::OFFLINE;
    constexpr static auto COMPRESSED = Underlying::COMPRESSED;
    constexpr static auto ABORT_ON_ERROR = Underlying::ABORT_ON_ERROR;
    constexpr static auto AUTO_CONVERGE = Underlying::AUTO_CONVERGE;
    constexpr static auto RDMA_PIN_ALL = Underlying::RDMA_PIN_ALL;
    constexpr static auto POSTCOPY = Underlying::POSTCOPY;
    constexpr static auto TLS = Underlying::TLS;
};

}