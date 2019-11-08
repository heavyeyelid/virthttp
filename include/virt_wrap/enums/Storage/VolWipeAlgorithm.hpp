#ifndef VIRTPP_ENUM_STORAGE_VOLWIPEALGORITHM_HPP
#define VIRTPP_ENUM_STORAGE_VOLWIPEALGORITHM_HPP

#include "../../StorageVol.hpp"
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-storage.h>

namespace virt {
class StorageVol::WipeAlgorithm : private VirtEnumStorage<virStorageVolWipeAlgorithm>,
                                  public VirtEnumBase<WipeAlgorithm>,
                                  public EnumHelper<WipeAlgorithm> {
    friend VirtEnumBase<WipeAlgorithm>;
    friend EnumHelper<WipeAlgorithm>;
    enum class Underlying {
        ZERO = VIR_STORAGE_VOL_WIPE_ALG_ZERO,             /* 1-pass, all zeroes */
        NNSA = VIR_STORAGE_VOL_WIPE_ALG_NNSA,             /* 4-pass  NNSA Policy Letter NAP-14.1-C (XVI-8) */
        DOD = VIR_STORAGE_VOL_WIPE_ALG_DOD,               /* 4-pass DoD 5220.22-M section 8-306 procedure */
        BSI = VIR_STORAGE_VOL_WIPE_ALG_BSI,               /* 9-pass method recommended by the German Center of Security in Information Technologies */
        GUTMANN = VIR_STORAGE_VOL_WIPE_ALG_GUTMANN,       /* The canonical 35-pass sequence */
        SCHNEIER = VIR_STORAGE_VOL_WIPE_ALG_SCHNEIER,     /* 7-pass method described by Bruce Schneier in "Applied Cryptography" (1996) */
        PFITZNER7 = VIR_STORAGE_VOL_WIPE_ALG_PFITZNER7,   /* 7-pass random */
        PFITZNER33 = VIR_STORAGE_VOL_WIPE_ALG_PFITZNER33, /* 33-pass random */
        RANDOM = VIR_STORAGE_VOL_WIPE_ALG_RANDOM,         /* 1-pass random */
        TRIM = VIR_STORAGE_VOL_WIPE_ALG_TRIM,             /* 1-pass, trim all data on the volume by using TRIM or DISCARD */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"zero", "nnsa", "dod", "bsi", "gutmann", "schneier", "pfitzner7", "pfitzner33", "random", "trim"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto ZERO = Underlying::ZERO;
    constexpr static auto NNSA = Underlying::NNSA;
    constexpr static auto DOD = Underlying::DOD;
    constexpr static auto BSI = Underlying::BSI;
    constexpr static auto GUTMANN = Underlying::GUTMANN;
    constexpr static auto SCHNEIER = Underlying::SCHNEIER;
    constexpr static auto PFITZNER7 = Underlying::PFITZNER7;
    constexpr static auto PFITZNER33 = Underlying::PFITZNER33;
    constexpr static auto RANDOM = Underlying::RANDOM;
    constexpr static auto TRIM = Underlying::TRIM;
};

} // namespace virt

#endif