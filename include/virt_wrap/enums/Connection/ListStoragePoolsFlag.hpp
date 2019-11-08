#pragma once
#include <libvirt/libvirt.h>
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"

namespace virt {

class Connection::List::StoragePool::Flag : private VirtEnumStorage<virConnectListAllStoragePoolsFlags>,
                                            public VirtEnumBase<Flag>,
                                            public EnumSetHelper<Flag> {
    friend VirtEnumBase<Flag>;
    friend EnumSetHelper<Flag>;
    enum class Underlying : unsigned {
        DEFAULT = 0,
        INACTIVE = VIR_CONNECT_LIST_STORAGE_POOLS_INACTIVE,
        ACTIVE = VIR_CONNECT_LIST_STORAGE_POOLS_ACTIVE,

        PERSISTENT = VIR_CONNECT_LIST_STORAGE_POOLS_PERSISTENT,
        TRANSIENT = VIR_CONNECT_LIST_STORAGE_POOLS_TRANSIENT,

        AUTOSTART = VIR_CONNECT_LIST_STORAGE_POOLS_AUTOSTART,
        NO_AUTOSTART = VIR_CONNECT_LIST_STORAGE_POOLS_NO_AUTOSTART,

        /* List pools by type */
        DIR = VIR_CONNECT_LIST_STORAGE_POOLS_DIR,
        FS = VIR_CONNECT_LIST_STORAGE_POOLS_FS,
        NETFS = VIR_CONNECT_LIST_STORAGE_POOLS_NETFS,
        LOGICAL = VIR_CONNECT_LIST_STORAGE_POOLS_LOGICAL,
        DISK = VIR_CONNECT_LIST_STORAGE_POOLS_DISK,
        ISCSI = VIR_CONNECT_LIST_STORAGE_POOLS_ISCSI,
        SCSI = VIR_CONNECT_LIST_STORAGE_POOLS_SCSI,
        MPATH = VIR_CONNECT_LIST_STORAGE_POOLS_MPATH,
        RBD = VIR_CONNECT_LIST_STORAGE_POOLS_RBD,
        SHEEPDOG = VIR_CONNECT_LIST_STORAGE_POOLS_SHEEPDOG,
        GLUSTER = VIR_CONNECT_LIST_STORAGE_POOLS_GLUSTER,
        ZFS = VIR_CONNECT_LIST_STORAGE_POOLS_ZFS,
        VSTORAGE = VIR_CONNECT_LIST_STORAGE_POOLS_VSTORAGE,
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"inactive",
                                          "active"
                                          "persistent",
                                          "transient",
                                          "autostart",
                                          "no_autostart",
                                          "dir",
                                          "netfs",
                                          "logical",
                                          "disk",
                                          "iscsi",
                                          "scsi",
                                          "mpath",
                                          "rbd",
                                          "sheepdog",
                                          "gluster",
                                          "zfs",
                                          "vstorage"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto DEFAULT = Underlying::DEFAULT;
    constexpr static auto INACTIVE = Underlying::INACTIVE;
    constexpr static auto ACTIVE = Underlying::ACTIVE;
    constexpr static auto PERSISTENT = Underlying::PERSISTENT;
    constexpr static auto TRANSIENT = Underlying::TRANSIENT;
    constexpr static auto AUTOSTART = Underlying::AUTOSTART;
    constexpr static auto NO_AUTOSTART = Underlying::NO_AUTOSTART;
    constexpr static auto DIR = Underlying::DIR;
    constexpr static auto FS = Underlying::FS;
    constexpr static auto NETFS = Underlying::NETFS;
    constexpr static auto LOGICAL = Underlying::LOGICAL;
    constexpr static auto DISK = Underlying::DISK;
    constexpr static auto ISCSI = Underlying::ISCSI;
    constexpr static auto SCSI = Underlying::SCSI;
    constexpr static auto MPATH = Underlying::MPATH;
    constexpr static auto RBD = Underlying::RBD;
    constexpr static auto SHEEPDOG = Underlying::SHEEPDOG;
    constexpr static auto GLUSTER = Underlying::GLUSTER;
    constexpr static auto ZFS = Underlying::ZFS;
    constexpr static auto VSTORAGE = Underlying::VSTORAGE;
};
}