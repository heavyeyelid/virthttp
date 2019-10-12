#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {
class Domain::BlockCommitFlag : private VirtEnumStorage<virDomainBlockCommitFlags>,
                                public VirtEnumBase<BlockCommitFlag>,
                                public EnumSetHelper<BlockCommitFlag> {
    friend VirtEnumBase<BlockCommitFlag>;
    friend EnumSetHelper<BlockCommitFlag>;
    enum class Underlying {
        SHALLOW = VIR_DOMAIN_BLOCK_COMMIT_SHALLOW,   /* nullptr base means next backing file, not whole chain */
        DELETE = VIR_DOMAIN_BLOCK_COMMIT_DELETE,     /* Delete any files that are now invalid after their contents have been committed */
        ACTIVE = VIR_DOMAIN_BLOCK_COMMIT_ACTIVE,     /* Allow a two-phase commit when top is the active layer */
        RELATIVE = VIR_DOMAIN_BLOCK_COMMIT_RELATIVE, /* keep the backing chain referenced using relative names */
        BANDWIDTH_BYTES = VIR_DOMAIN_BLOCK_COMMIT_BANDWIDTH_BYTES, /* bandwidth in bytes/s instead of MiB/s */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"shallow", "delete", "active", "relative", "bandwitdh_bytes"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto SHALLOW = Underlying::SHALLOW;
    constexpr static auto DELETE = Underlying::DELETE;
    constexpr static auto ACTIVE = Underlying::ACTIVE;
    constexpr static auto RELATIVE = Underlying::RELATIVE;
    constexpr static auto BANDWIDTH_BYTES = Underlying::BANDWIDTH_BYTES;
};

} // namespace virt
