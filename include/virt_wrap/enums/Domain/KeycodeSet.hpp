#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::KeycodeSet : private VirtEnumStorage<virKeycodeSet>, public VirtEnumBase<KeycodeSet>, public EnumHelper<KeycodeSet> {
    friend VirtEnumBase<KeycodeSet>;
    friend EnumHelper<KeycodeSet>;
    enum class Underlying {
        LINUX = VIR_KEYCODE_SET_LINUX,
        XT = VIR_KEYCODE_SET_XT,
        ATSET1 = VIR_KEYCODE_SET_ATSET1,
        ATSET2 = VIR_KEYCODE_SET_ATSET2,
        ATSET3 = VIR_KEYCODE_SET_ATSET3,
        OSX = VIR_KEYCODE_SET_OSX,
        KBD = VIR_KEYCODE_SET_XT_KBD,
        USB = VIR_KEYCODE_SET_USB,
        WIN32 = VIR_KEYCODE_SET_WIN32,
        QNUM = VIR_KEYCODE_SET_QNUM,
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {"linux", "xt", "atset1", "atset2", "atset3", "osx", ""};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto LINUX = Underlying::LINUX;
    constexpr static auto XT = Underlying::XT;
    constexpr static auto ATSET1 = Underlying::ATSET1;
    constexpr static auto ATSET2 = Underlying::ATSET2;
    constexpr static auto ATSET3 = Underlying::ATSET3;
    constexpr static auto OSX = Underlying::OSX;
    constexpr static auto KBD = Underlying::KBD;
    constexpr static auto USB = Underlying::USB;
    constexpr static auto WIN32 = Underlying::WIN32;
    constexpr static auto QNUM = Underlying::QNUM;
};

}