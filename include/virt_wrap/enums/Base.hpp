#pragma once
#include <type_traits>
#include "virt_wrap/utility.hpp"

template <class Storage> struct VirtEnumStorage { Storage underlying{}; };

template <class CRTP> class VirtEnumBase {
    constexpr auto& get_underlying() noexcept { return static_cast<CRTP&>(*this).underlying; }
    constexpr auto& get_underlying() const noexcept { return static_cast<const CRTP&>(*this).underlying; }

    template <class...> friend class EnumSetTie;

  public:
    constexpr VirtEnumBase() noexcept { get_underlying() = {}; };
    template <class T> constexpr VirtEnumBase(EHTag, T e) noexcept {
        static_assert(std::is_integral_v<T>);
        get_underlying() = std::remove_reference_t<decltype(get_underlying())>(e);
    }
    template <class E> constexpr /* explicit(false) */ VirtEnumBase(E e) noexcept {
        static_assert(std::is_same_v<std::add_const_t<E>, std::remove_reference_t<decltype(CRTP::default_value)>>);
        get_underlying() = std::remove_reference_t<decltype(get_underlying())>(e);
    }
    template <class E> constexpr /* explicit(false) */ CRTP& operator=(E e) noexcept {
        static_assert(std::is_same_v<std::add_const_t<E>, std::remove_reference_t<decltype(CRTP::default_value)>>);
        get_underlying() = std::remove_reference_t<decltype(get_underlying())>(e);
        return static_cast<CRTP&>(*this);
    }

    template <class E, std::enable_if_t<std::is_base_of_v<VirtEnumBase<E>, E>, int>> friend constexpr auto to_integral(E e) noexcept;
};

template <class E, std::enable_if_t<std::is_base_of_v<VirtEnumBase<E>, E>, int> = 0> constexpr auto to_integral(E e) noexcept {
    return to_integral(static_cast<const VirtEnumBase<E>&>(e).get_underlying());
}

template <class U, class V, class = std::enable_if_t<std::is_base_of_v<VirtEnumBase<U>, U> && std::is_enum_v<V>>>
constexpr bool operator==(U lhs, V rhs) noexcept {
    return to_integral(lhs) == to_integral(rhs);
}
template <class U, class V, class = std::enable_if_t<std::is_base_of_v<VirtEnumBase<U>, U> && std::is_enum_v<V>>>
constexpr bool operator!=(U lhs, V rhs) noexcept {
    return to_integral(lhs) != to_integral(rhs);
}

class VirtCustomEnum {};

template <class... Es> class EnumSetTie {
    using Underlying = std::common_type_t<decltype(static_cast<VirtEnumBase<Es>&>(std::declval<Es&>()).get_underlying())...>;
    Underlying underlying;

  public:
    template <class E, class = std::enable_if_t<std::disjunction_v<std::is_convertible_v<E, Underlying>, std::is_same_v<E, Es>...>>>
    constexpr explicit EnumSetTie(E v) : underlying(v) {}
    template <class E, class = std::enable_if_t<std::disjunction_v<std::is_convertible_v<E, Underlying>, std::is_same_v<E, Es>...>>>
    constexpr EnumSetTie& operator=(E v) noexcept {
        underlying = v;
        return *this;
    }
    friend constexpr auto to_integral(EnumSetTie est) { return est.underlying; }

    constexpr static auto from_string(std::string_view sv) {
        std::optional<EnumSetTie<Es...>> ret{};
        visit(std::tuple<Es...>{}, [=, &ret](auto ec) {
            if (const auto res = decltype(ec)::from_string(sv); res)
                ret = *res;
        });
        return ret;
    }
};

template <class... Es> constexpr EnumSetTie<Es...> operator|(EnumSetTie<Es...> lhs, EnumSetTie<Es...> rhs) noexcept {
    return {to_integral(lhs) | to_integral(rhs)};
}
template <class E, class... Es, class = std::enable_if_t<std::disjunction_v<std::is_same_v<E, Es>...>>>
constexpr EnumSetTie<Es...> operator|(EnumSetTie<Es...> lhs, E rhs) noexcept {
    return {to_integral(lhs) | to_integral(rhs)};
}
template <class E, class... Es, class = std::enable_if_t<std::disjunction_v<std::is_same_v<E, Es>...>>>
constexpr EnumSetTie<Es...> operator|(E lhs, EnumSetTie<Es...> rhs) noexcept {
    return {to_integral(lhs) | to_integral(rhs)};
}
template <class... Es> constexpr EnumSetTie<Es...>& operator|=(EnumSetTie<Es...>& lhs, EnumSetTie<Es...> rhs) noexcept {
    return lhs = {to_integral(lhs) | to_integral(rhs)};
}
template <class E, class... Es, class = std::enable_if_t<std::disjunction_v<std::is_same_v<E, Es>...>>>
constexpr EnumSetTie<Es...>& operator|=(EnumSetTie<Es...>& lhs, E rhs) noexcept {
    return lhs = {to_integral(lhs) | to_integral(rhs)};
}