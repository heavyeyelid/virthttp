#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::XmlFlag : private VirtEnumStorage<virDomainXMLFlags>, public VirtEnumBase<XmlFlag>, public EnumSetHelper<XmlFlag> {
    friend VirtEnumBase<XmlFlag>;
    friend EnumSetHelper<XmlFlag>;
    enum class Underlying {
        DEFAULT = 0,
        SECURE = VIR_DOMAIN_XML_SECURE,         /* dump security sensitive information too */
        INACTIVE = VIR_DOMAIN_XML_INACTIVE,     /* dump inactive domain information */
        UPDATE_CPU = VIR_DOMAIN_XML_UPDATE_CPU, /* update guest CPU requirements according to host CPU */
        MIGRATABLE = VIR_DOMAIN_XML_MIGRATABLE, /* dump XML suitable for migration */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"secure", "inactive", "update_cpu", "migratable"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto DEFAULT = Underlying::DEFAULT;
    constexpr static auto SECURE = Underlying::SECURE;
    constexpr static auto INACTIVE = Underlying::INACTIVE;
    constexpr static auto UPDATE_CPU = Underlying::UPDATE_CPU;
    constexpr static auto MIGRATABLE = Underlying::MIGRATABLE;
};

}