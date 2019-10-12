#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::MetadataType : private VirtEnumStorage<virDomainMetadataType>, public VirtEnumBase<MetadataType>, public EnumHelper<MetadataType> {
    friend VirtEnumBase<MetadataType>;
    friend EnumHelper<MetadataType>;
    enum class Underlying {
        DESCRIPTION = VIR_DOMAIN_METADATA_DESCRIPTION, /* Operate on <description> */
        TITLE = VIR_DOMAIN_METADATA_TITLE,             /* Operate on <title> */
        ELEMENT = VIR_DOMAIN_METADATA_ELEMENT,         /* Operate on <metadata> */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"description", "title", "element"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto DESCRIPTION = Underlying::DESCRIPTION;
    constexpr static auto TITLE = Underlying::TITLE;
    constexpr static auto ELEMENT = Underlying::ELEMENT;
};

}