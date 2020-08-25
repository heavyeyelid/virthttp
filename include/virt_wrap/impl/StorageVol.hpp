#ifndef VIRTPP_IMPL_STORAGEVOL_HPP
#define VIRTPP_IMPL_STORAGEVOL_HPP

#include <utility>
#include <libvirt/libvirt.h>
#include "../StorageVol.hpp"
#include "../Stream.hpp"

namespace virt {

inline StorageVol::StorageVol(StorageVol&& oth) noexcept : underlying(oth.underlying) { oth.underlying = nullptr; }
inline StorageVol& StorageVol::operator=(StorageVol&& oth) noexcept {
    std::swap(underlying, oth.underlying);
    return *this;
}
inline StorageVol::~StorageVol() noexcept { virStorageVolFree(underlying); }

constexpr inline StorageVol::operator bool() const noexcept { return underlying != nullptr; }

inline bool StorageVol::delete_(DeleteFlag flags) noexcept { return virStorageVolDelete(underlying, to_integral(flags)) == 0; }
inline bool StorageVol::download(Stream& strm, unsigned long long offset, unsigned long long length, DownloadFlag flags) noexcept {
    return virStorageVolDownload(underlying, strm.underlying, offset, length, to_integral(flags));
}
inline Connection StorageVol::getConnect() const noexcept {
    const auto res = virStorageVolGetConnect(underlying);
    if (res)
        virConnectRef(res);
    return Connection{res};
}

inline auto StorageVol::getInfo() const noexcept -> std::optional<Info> {
    std::optional<Info> ret;
    return (virStorageVolGetInfo(underlying, &ret.emplace()) == 0) ? ret : std::nullopt;
}
inline auto StorageVol::getInfo(InfoFlag flags) const noexcept -> std::optional<Info> {
    std::optional<Info> ret;
    return (virStorageVolGetInfoFlags(underlying, &ret.emplace(), to_integral(flags)) == 0) ? ret : std::nullopt;
}

inline passive<gsl::czstring<>> StorageVol::getKey() const noexcept { return virStorageVolGetKey(underlying); }
inline passive<gsl::czstring<>> StorageVol::getName() const noexcept { return virStorageVolGetName(underlying); }
inline UniqueZstring StorageVol::getPath() const noexcept { return UniqueZstring{virStorageVolGetPath(underlying)}; }
inline StoragePool StorageVol::getPool() const noexcept { return StoragePool{virStoragePoolLookupByVolume(underlying)}; }
inline UniqueZstring StorageVol::getXMLDesc() const noexcept { return UniqueZstring{virStorageVolGetXMLDesc(underlying, 0)}; }
inline bool StorageVol::resize(unsigned long long capacity, ResizeFlag flags) noexcept {
    return virStorageVolResize(underlying, capacity, to_integral(flags)) == 0;
}
inline bool StorageVol::upload(Stream& stream, unsigned long long offset, unsigned long long length, UploadFlag flags) noexcept {
    return virStorageVolUpload(underlying, stream.underlying, offset, length, to_integral(flags)) == 0;
}
inline bool StorageVol::wipe() noexcept { return virStorageVolWipe(underlying, 0) == 0; }
inline bool StorageVol::wipePattern(WipeAlgorithm algorithm) noexcept { return virStorageVolWipePattern(underlying, to_integral(algorithm), 0) == 0; }

inline StorageVol StorageVol::createXML(StoragePool& pool, gsl::czstring<> xmlDesc, CreateFlag flags) noexcept {
    return StorageVol{virStorageVolCreateXML(pool.underlying, xmlDesc, to_integral(flags))};
}
inline StorageVol StorageVol::createXMLFrom(StoragePool& pool, gsl::czstring<> xmlDesc, const StorageVol& clonevol, CreateFlag flags) noexcept {
    return StorageVol{virStorageVolCreateXMLFrom(pool.underlying, xmlDesc, clonevol.underlying, to_integral(flags))};
}

} // namespace virt

#endif