#ifndef VIRTPP_STORAGEPOOL_HPP
#define VIRTPP_STORAGEPOOL_HPP

#include <gsl/gsl>
#include <libvirt/libvirt.h>
#include "fwd.hpp"
#include "tfe.hpp"

namespace virt {

struct StoragePool {
  private:
    friend Connection;
    friend StorageVol;
    virStoragePoolPtr underlying = nullptr;

  public:
    class BuildFlag;
    class CreateFlag;
    class DeleteFlag;
    class State;

    class Info;

    constexpr StoragePool() noexcept = default;
    constexpr StoragePool(const StoragePool&) noexcept = delete;
    inline StoragePool(StoragePool&&) noexcept;
    constexpr StoragePool& operator=(const StoragePool&) noexcept = delete;
    inline StoragePool& operator=(StoragePool&&) noexcept;
    inline ~StoragePool() noexcept;

    constexpr explicit StoragePool(virStoragePoolPtr ptr) noexcept : underlying{ptr} {}
    [[nodiscard]] constexpr inline explicit operator bool() const noexcept;

    inline bool build(BuildFlag flags) noexcept;
    inline bool create(CreateFlag flags) noexcept;
    inline bool delete_(DeleteFlag flags) noexcept;
    inline bool destroy() noexcept;
    [[nodiscard]] inline TFE getAutostart() const noexcept;
    [[nodiscard]] inline Connection getConnect() const noexcept;
    [[nodiscard]] inline auto getInfo() const noexcept -> std::optional<Info>;
    [[nodiscard]] inline passive<gsl::czstring<>> getName() const noexcept;
    [[nodiscard]] auto getUUID() const -> std::optional<std::array<unsigned char, VIR_UUID_BUFLEN>>;
    [[nodiscard]] auto getUUIDString() const noexcept -> std::optional<std::array<char, VIR_UUID_STRING_BUFLEN>>;
    [[nodiscard]] inline passive<gsl::czstring<>> getXMLDesc() const noexcept;
    [[nodiscard]] inline TFE isActive() const noexcept;
    [[nodiscard]] inline TFE isPersistent() const noexcept;
    [[nodiscard]] inline auto listAllVolumes() const noexcept;
    [[nodiscard]] inline auto extractAllVolumes() const;
    [[nodiscard]] inline auto listVolumesNames() const noexcept;
    [[nodiscard]] inline int numOfVolumes() const noexcept;
    inline bool refresh() noexcept;
    inline bool setAutostart(bool autostart) noexcept;
    inline bool undefine() noexcept;

    [[nodiscard]] static inline StoragePool createXML(Connection& conn, gsl::czstring<> xml, CreateFlag flags);
    [[nodiscard]] static inline StoragePool defineXML(Connection& conn, gsl::czstring<> xml);

    [[nodiscard]] inline StorageVol volLookupByName(gsl::czstring<> name) const noexcept;
};

} // namespace virt

#include "enums/Storage/StoragePool.hpp"

namespace virt {
struct StoragePool::Info : private virStoragePoolInfo {
    using Base = virStoragePoolInfo;
    friend StoragePool;
    [[nodiscard]] constexpr State state() const noexcept { return State{EHTag{}, Base::state}; }
    [[nodiscard]] State& state() noexcept { return reinterpret_cast<State&>(Base::state); }
    [[nodiscard]] constexpr unsigned long long capacity() const noexcept { return Base::capacity; }
    [[nodiscard]] constexpr unsigned long long& capacity() noexcept { return Base::capacity; }
    [[nodiscard]] constexpr unsigned long long allocation() const noexcept { return Base::allocation; }
    [[nodiscard]] constexpr unsigned long long& allocation() noexcept { return Base::allocation; }
    [[nodiscard]] constexpr unsigned long long available() const noexcept { return Base::available; }
    [[nodiscard]] constexpr unsigned long long& available() noexcept { return Base::available; }
};
} // namespace virt

#include "impl/StoragePool.hpp"

#endif