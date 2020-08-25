//
// Created by hugo on 5/15/20.
//

#pragma once

namespace virt::enums::network {
class XMLFlags : private VirtEnumStorage<virDomainXMLFlags>, public VirtEnumBase<XMLFlags>, public EnumSetHelper<XMLFlags> {
    friend VirtEnumBase<XMLFlags>;
    friend EnumSetHelper<XMLFlags>;
    enum class Underlying {
        DEFAULT = 0,
        INACTIVE = VIR_NETWORK_XML_INACTIVE,
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"secure", "inactive", "update_cpu", "migratable"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumSetHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto DEFAULT = Underlying::DEFAULT;
    constexpr static auto INACTIVE = Underlying::INACTIVE;
};
} // namespace virt::enums::network
