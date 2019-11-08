#ifndef VIRTPP_STORAGEVOL_HPP
#define VIRTPP_STORAGEVOL_HPP

#include <gsl/gsl>
#include <libvirt/libvirt-storage.h>
#include "fwd.hpp"

namespace virt {

struct StorageVol {
  private:
    friend Connection;
    friend StoragePool;

    virStorageVolPtr underlying = nullptr;

  public:
    class CreateFlag;
    class DeleteFlag;
    class DownloadFlag;
    class InfoFlag;
    class ResizeFlag;
    class Type;
    class UploadFlag;
    class WipeAlgorithm;

    class Info;

    constexpr StorageVol() noexcept = default;
    constexpr StorageVol(const StorageVol&) noexcept = delete;
    inline StorageVol(StorageVol&&) noexcept;
    constexpr StorageVol& operator=(const StorageVol&) noexcept = delete;
    inline StorageVol& operator=(StorageVol&&) noexcept;
    inline ~StorageVol() noexcept;

    constexpr explicit StorageVol(virStorageVolPtr ptr) noexcept : underlying{ptr} {}
    [[nodiscard]] constexpr inline explicit operator bool() const noexcept;

    inline bool delete_(DeleteFlag flags) noexcept;
    inline bool download(Stream& strm, unsigned long long offset, unsigned long long length, DownloadFlag flags) noexcept;
    [[nodiscard]] inline Connection getConnect() const noexcept;
    [[nodiscard]] inline auto getInfo() const noexcept -> std::optional<Info>;
    [[nodiscard]] inline auto getInfo(InfoFlag flags) const noexcept -> std::optional<Info>;
    [[nodiscard]] inline passive<gsl::czstring<>> getKey() const noexcept;
    [[nodiscard]] inline passive<gsl::czstring<>> getName() const noexcept;
    [[nodiscard]] inline UniqueZstring getPath() const noexcept;
    [[nodiscard]] inline StoragePool getPool() const noexcept;
    [[nodiscard]] inline UniqueZstring getXMLDesc() const noexcept;
    inline bool resize(unsigned long long capacity, ResizeFlag flags) noexcept;
    inline bool upload(Stream& stream, unsigned long long offset, unsigned long long length, UploadFlag flags) noexcept;
    inline bool wipe() noexcept;
    inline bool wipePattern(WipeAlgorithm algorithm) noexcept;

    [[nodiscard]] static inline StorageVol createXML(StoragePool& pool, gsl::czstring<> xmlDesc, CreateFlag flags) noexcept;
    [[nodiscard]] static inline StorageVol createXMLFrom(StoragePool& pool, gsl::czstring<> xmlDesc, const StorageVol& clonevol,
                                                         CreateFlag flags) noexcept;
};

} // namespace virt

#include "enums/Storage/StorageVol.hpp"

namespace virt {
struct StorageVol::Info : private virStorageVolInfo {
    using Base = virStorageVolInfo;
    friend StorageVol;
    [[nodiscard]] constexpr Type type() const noexcept { return Type{EHTag{}, Base::type}; }
    [[nodiscard]] Type& type() noexcept { return reinterpret_cast<Type&>(Base::type); }
    [[nodiscard]] constexpr unsigned long long capacity() const noexcept { return Base::capacity; }
    [[nodiscard]] constexpr unsigned long long& capacity() noexcept { return Base::capacity; }
    [[nodiscard]] constexpr unsigned long long allocation() const noexcept { return Base::allocation; }
    [[nodiscard]] constexpr unsigned long long& allocation() noexcept { return Base::allocation; }
};
} // namespace virt

#include "impl/StorageVol.hpp"

#endif