#pragma once
#include <type_traits>

namespace nstd {

namespace detail {
template <class Default, class AlwaysVoid, template <class...> class Op, class... Args> struct detector {
    using value_t = std::false_type;
    using type = Default;
};

template <class Default, template <class...> class Op, class... Args> struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
    using type = Op<Args...>;
};

} // namespace detail

class nonesuch;

template <template <class...> class Op, class... Args> using is_detected = typename detail::detector<nonesuch, void, Op, Args...>::value_t;

template <template <class...> class Op, class... Args> constexpr bool is_detected_v = is_detected<Op, Args...>::value;

} // namespace nstd
