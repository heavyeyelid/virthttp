#ifndef VIRTPP_IMPL_STORAGEPOOL_HPP
#define VIRTPP_IMPL_STORAGEPOOL_HPP

#include <utility>
#include <libvirt/libvirt.h>
#include "../StoragePool.hpp"
#include "../StorageVol.hpp"

namespace virt {

inline StoragePool::StoragePool(StoragePool&& oth) noexcept : underlying(oth.underlying) { oth.underlying = nullptr; }
inline StoragePool& StoragePool::operator=(StoragePool&& oth) noexcept {
    std::swap(underlying, oth.underlying);
    return *this;
}
inline StoragePool::~StoragePool() noexcept { virStoragePoolFree(underlying); }

constexpr inline StoragePool::operator bool() const noexcept { return underlying != nullptr; }

inline bool StoragePool::build(BuildFlag flags) noexcept { return virStoragePoolBuild(underlying, to_integral(flags)) == 0; }
inline bool StoragePool::create(CreateFlag flags) noexcept { return virStoragePoolCreate(underlying, to_integral(flags)) == 0; }
inline bool StoragePool::delete_(DeleteFlag flags) noexcept { return virStoragePoolDelete(underlying, to_integral(flags)) == 0; }
inline bool StoragePool::destroy() noexcept { return virStoragePoolDestroy(underlying) == 0; }

[[nodiscard]] inline TFE StoragePool::getAutostart() const noexcept {
    int val = 0;
    const auto res = virStoragePoolGetAutostart(underlying, &val);
    return TFE{res + val};
}

[[nodiscard]] inline Connection StoragePool::getConnect() const noexcept {
    const auto ret = virStoragePoolGetConnect(underlying);
    if (ret)
        virConnectRef(ret);
    return Connection{ret};
}

[[nodiscard]] inline auto StoragePool::getInfo() const noexcept -> std::optional<Info> {
    std::optional<Info> ret;
    return virStoragePoolGetInfo(underlying, &ret.emplace()) == 0 ? ret : std::nullopt;
}

[[nodiscard]] inline gsl::czstring<> StoragePool::getName() const noexcept { return virStoragePoolGetName(underlying); }

[[nodiscard]] inline auto StoragePool::getUUID() const -> std::optional<std::array<unsigned char, VIR_UUID_BUFLEN>> {
    std::optional<std::array<unsigned char, VIR_UUID_BUFLEN>> ret{};
    return virStoragePoolGetUUID(underlying, ret.emplace().data()) == 0 ? ret : std::nullopt;
}

[[nodiscard]] inline auto StoragePool::getUUIDString() const noexcept -> std::optional<std::array<char, VIR_UUID_STRING_BUFLEN>> {
    std::optional<std::array<char, VIR_UUID_STRING_BUFLEN>> ret{};
    return virStoragePoolGetUUIDString(underlying, ret.emplace().data()) == 0 ? ret : std::nullopt;
}

[[nodiscard]] inline passive<gsl::czstring<>> StoragePool::getXMLDesc() const noexcept { return virStoragePoolGetXMLDesc(underlying, 0); }

[[nodiscard]] inline TFE StoragePool::isActive() const noexcept { return TFE{virStoragePoolIsActive(underlying)}; }
[[nodiscard]] inline TFE StoragePool::isPersistent() const noexcept { return TFE{virStoragePoolIsPersistent(underlying)}; }
[[nodiscard]] inline auto StoragePool::listAllVolumes() const noexcept {
    return meta::light::wrap_opram_owning_set_destroyable_arr<StorageVol>(underlying, virStoragePoolListAllVolumes, 0u);
}
[[nodiscard]] inline auto StoragePool::extractAllVolumes() const {
    return meta::heavy::wrap_opram_owning_set_destroyable_arr<StorageVol>(underlying, virStoragePoolListAllVolumes, 0u);
}
[[nodiscard]] inline auto StoragePool::listVolumesNames() const noexcept {
    return meta::light::wrap_oparm_owning_fill_freeable_arr(underlying, virStoragePoolNumOfVolumes, virStoragePoolListVolumes);
}
[[nodiscard]] inline int StoragePool::numOfVolumes() const noexcept { return virStoragePoolNumOfVolumes(underlying); }
inline bool StoragePool::refresh() noexcept { return virStoragePoolRefresh(underlying, 0) == 0; }
inline bool StoragePool::setAutostart(bool autostart) noexcept { return virStoragePoolSetAutostart(underlying, +autostart) == 0; }
inline bool StoragePool::undefine() noexcept { return virStoragePoolUndefine(underlying) == 0; }

inline StoragePool StoragePool::createXML(Connection& conn, gsl::czstring<> xml, CreateFlag flags) {
    return StoragePool{virStoragePoolCreateXML(conn.underlying, xml, to_integral(flags))};
}
inline StoragePool StoragePool::defineXML(Connection& conn, gsl::czstring<> xml) {
    return StoragePool{virStoragePoolDefineXML(conn.underlying, xml, 0)};
}

inline StorageVol StoragePool::volLookupByName(gsl::czstring<> name) const noexcept {
    return StorageVol{virStorageVolLookupByName(underlying, name)};
}

} // namespace virt

#endif