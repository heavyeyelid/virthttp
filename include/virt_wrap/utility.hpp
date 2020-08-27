//
// Created by _as on 2019-01-31.
//

#pragma once

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <type_traits>
#include <gsl/gsl>
#include "cexpr_algs.hpp"

#ifdef __GNUC__
#define UNREACHABLE __builtin_unreachable()
#elif _MSC_VER
#define UNREACHABLE __assume(0)
#else
#define UNREACHABLE
#endif

template <typename E, std::enable_if_t<std::is_enum_v<E>, int> = 0> constexpr inline decltype(auto) to_integral(E e) {
    return static_cast<typename std::underlying_type<E>::type>(e);
}
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0> constexpr inline decltype(auto) to_integral(T v) { return v; }

class Empty {};

template <class> struct RemoveOptional;

template <class T> struct RemoveOptional<std::optional<T>> { using type = T; };

template <class OptT> using RemoveOptional_t = typename RemoveOptional<OptT>::type;

template <class... Ts> void sink(Ts&&... ts) { (static_cast<void>(std::move(ts)), ...); }

template <class Lambda, class... Ts> constexpr auto test_sfinae(Lambda lambda, Ts&&...) -> decltype(lambda(std::declval<Ts>()...), bool{}) {
    return true;
}
constexpr bool test_sfinae(...) { return false; }

template <typename T, typename V, size_t... I> void visit_impl(T&& t, V&& v, std::index_sequence<I...>) { (..., v(std::get<I>(t))); }

template <typename T, typename V> void visit(T&& t, V&& v) {
    visit_impl(std::forward<T>(t), std::forward<V>(v), std::make_index_sequence<std::tuple_size<typename std::decay<T>::type>::value>());
}

template <typename T> using passive = T;

template <typename T> inline void freeany(T ptr) {
    static_assert(std::is_array_v<T> || std::is_pointer_v<T>, "T needs to be a pointer or an array type");
    std::free(ptr);
}

class EHTag {};

template <class CRTP> class EnumHelper {
    constexpr auto& get_underlying(EHTag) const noexcept { return static_cast<const CRTP&>(*this).underlying; }
    constexpr auto& values() const noexcept { return static_cast<const CRTP&>(*this).values; }

    constexpr static EHTag tag{};

  public:
    [[nodiscard]] constexpr std::string_view to_string() const noexcept { return values()[to_integral(get_underlying(tag))]; }
    [[nodiscard]] constexpr auto from_string_base(std::string_view v) const noexcept {
        const auto res = cexpr::find(values().cbegin(), values().cend(), v);
        return res != values().end() ? std::optional{CRTP{tag, std::distance(values().cbegin(), res)}} : std::nullopt;
    }
};

template <class CRTP> class EnumSetHelper {
    constexpr auto& get_underlying(EHTag) const noexcept { return static_cast<const CRTP&>(*this).underlying; }
    constexpr auto& values() const noexcept { return static_cast<const CRTP&>(*this).values; }

    constexpr static EHTag tag{};

  public:
    [[nodiscard]] constexpr std::string_view to_string() const noexcept {
        return values()[sizeof(decltype(to_integral(get_underlying(tag)))) * 8 - __builtin_clz(to_integral(get_underlying(tag))) -
                        1]; // C++2a: use std::countl_zeroe();
    }
    [[nodiscard]] constexpr auto from_string_base(std::string_view v) const noexcept {
        const auto res = cexpr::find(values().cbegin(), values().end(), v);
        return res != values().cend() ? std::optional{CRTP{tag, (1u << std::distance(values().cbegin(), res))}} : std::nullopt;
    }
};

template <class E, class = std::enable_if_t<std::is_base_of_v<EnumSetHelper<E>, E>>> E operator|(E lhs, E rhs) {
    return {EHTag{}, to_integral(lhs) | to_integral(rhs)};
}

template <class E, class = std::enable_if_t<std::is_base_of_v<EnumSetHelper<E>, E>>> E& operator|=(E& lhs, E rhs) {
    return lhs = E{EHTag{}, to_integral(lhs) | to_integral(rhs)};
}

template <class E> class EnumSetIterator {
    using U = decltype(to_integral(std::declval<E>()));
    E e;

  public:
    explicit constexpr EnumSetIterator(E e) noexcept : e(e){};

    constexpr auto& operator++() noexcept {
        const auto lz = __builtin_clz(to_integral(e));
        const auto hi1b = sizeof(U) * 8 - lz - 1;
        if (hi1b == -1)
            e = E(0);
        else
            e = E(to_integral(e) &= ~(1 << hi1b));
        return *this;
    }

    constexpr E operator*() const noexcept {
        const auto lz = __builtin_clz(to_integral(e));
        const auto hi1b = sizeof(U) * 8 - lz - 1;
        return 1u << hi1b;
    }

    constexpr static EnumSetIterator end{E(0)};
};

class alignas(alignof(char*)) UniqueZstring {
    gsl::owner<char*> ptr{};

  public:
    constexpr UniqueZstring() noexcept = default;
    constexpr explicit UniqueZstring(gsl::owner<char*> ptr) : ptr(ptr) {}
    constexpr explicit UniqueZstring(const UniqueZstring&) noexcept = delete;
    inline UniqueZstring(UniqueZstring&& uz) noexcept : ptr(uz.ptr) { uz.ptr = nullptr; };
    inline ~UniqueZstring() noexcept { std::free(ptr); }

    constexpr UniqueZstring& operator=(const UniqueZstring&) noexcept = delete;
    inline UniqueZstring& operator=(UniqueZstring&& uz) noexcept {
        this->~UniqueZstring();
        ptr = uz.ptr;
        uz.ptr = nullptr;
        return *this;
    };

    [[nodiscard]] inline auto begin() const noexcept { return std::string_view{ptr}.begin(); }
    [[nodiscard]] inline auto end() const noexcept { return std::string_view{ptr}.end(); }
    [[nodiscard]] inline auto cbegin() const noexcept { return std::string_view{ptr}.cbegin(); }
    [[nodiscard]] inline auto cend() const noexcept { return std::string_view{ptr}.cend(); }

    constexpr inline explicit operator const char*() const noexcept { return ptr; }
    constexpr inline explicit operator char*() noexcept { return ptr; }

    constexpr inline explicit operator bool() const noexcept { return ptr; }
};

namespace ext {
/* Function traits: */
// https://functionalcpp.wordpress.com/2013/08/05/function-traits/

template <class F> struct function_traits;

// function pointer
template <class R, class... Args> struct function_traits<R (*)(Args...)> : public function_traits<R(Args...)> {};

template <class R, class... Args> struct function_traits<R(Args...)> {
    using return_type = R;

    static constexpr std::size_t arity = sizeof...(Args);

    template <std::size_t N> struct Arg {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
    };

    template <std::size_t N> using Arg_t = typename Arg<N>::type;
};

// member function pointer
template <class C, class R, class... Args> struct function_traits<R (C::*)(Args...)> : public function_traits<R(C&, Args...)> {};

// const member function pointer
template <class C, class R, class... Args> struct function_traits<R (C::*)(Args...) const> : public function_traits<R(C&, Args...)> {};

// member object pointer
template <class C, class R> struct function_traits<R(C::*)> : public function_traits<R(C&)> {};

// functor
template <class F> struct function_traits {
  private:
    using call_type = function_traits<decltype(&F::operator())>;

  public:
    using return_type = typename call_type::return_type;

    static constexpr std::size_t arity = call_type::arity - 1;

    template <std::size_t N> struct Arg {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = typename call_type::template Arg<N + 1>::type;
    };
    template <std::size_t N> using Arg_t = typename Arg<N>::type;
};

template <class F> struct function_traits<F&> : public function_traits<F> {};

template <class F> struct function_traits<F&&> : public function_traits<F> {};
} // namespace ext

/*                  */
template <typename T> struct NoallocWFree : private std::allocator<gsl::owner<T>> { // This clearly needs to be shadowed from the library users
    using value_type = gsl::owner<T>;
    using std::allocator<gsl::owner<T>>::allocate;
    using std::allocator<gsl::owner<T>>::deallocate;
    constexpr inline void construct(gsl::owner<T>* ptr) const noexcept {}
    inline void destroy(gsl::owner<T>* ptr) const noexcept { freeany(*ptr); }
};

template <typename T, typename D> struct UniqueSpan : std::unique_ptr<T[], D> {
    UniqueSpan(T* p, D d) noexcept : std::unique_ptr<T[], D>(p, d) {}
    inline auto begin() const noexcept { return this->get(); }
    inline auto end() const noexcept { return this->get() + this->get_deleter()(nullptr); }
    inline auto cbegin() const noexcept { return this->get(); }
    inline auto cend() const noexcept { return this->get() + this->get_deleter()(nullptr); }
};

template <typename T, typename D> struct UniqueNullTerminatedSpan : public std::unique_ptr<T[], D> {
    constexpr UniqueNullTerminatedSpan() noexcept = default;
    UniqueNullTerminatedSpan(T* p, D d) noexcept : std::unique_ptr<T[], D>(p, d) {}
    inline auto begin() const noexcept { return std::basic_string_view<T*>{this->get()}.begin(); }
    inline auto end() const noexcept { return std::basic_string_view<T*>{this->get()}.end(); }
    inline auto cbegin() const noexcept { return std::basic_string_view<T*>{this->get()}.cbegin(); }
    inline auto cend() const noexcept { return std::basic_string_view<T*>{this->get()}.cend(); }
};

template <typename T, typename D> struct UniqueFalseTerminatedSpan : public std::unique_ptr<T[], D> {
  private:
    static constexpr auto false_it(T* arr) {
        while (*arr)
            ++arr;
        return arr;
    }

  public:
    constexpr UniqueFalseTerminatedSpan() noexcept = default;
    UniqueFalseTerminatedSpan(T* p, D d) noexcept : std::unique_ptr<T[], D>(p, d) {}
    inline auto begin() const noexcept { return this->get(); }
    inline auto end() const noexcept { return false_it(this->get()); }
    inline auto cbegin() const noexcept { return this->get(); }
    inline auto cend() const noexcept { return false_it(this->get()); }
};

namespace virt::meta {
namespace impl::any {
// Need template lambdas to reduce bloat, as types need to be passed around
}
namespace light {
template <typename U, typename CF, typename DF> auto wrap_oparm_owning_fill_static_arr(U underlying, CF&& count_fcn, DF&& data_fcn) {
    using CountFTraits = ext::function_traits<CF>;
    static_assert(CountFTraits::arity == 1, "Counting function requires one argument");
    static_assert(std::is_same_v<typename CountFTraits::template Arg_t<0>, U>, "Counting function requires the underlying ptr as argument");
    using CountFRet = typename CountFTraits::return_type;

    using DataFTraits = ext::function_traits<DF>;
    static_assert(DataFTraits::arity == 3, "Data function requires three arguments");
    static_assert(std::is_same_v<typename DataFTraits::template Arg_t<0>, U>, "Data function requires the underlying ptr as first argument");
    static_assert(std::is_pointer_v<typename DataFTraits::template Arg_t<0>>, "Data function requires a pointer to the array as second argument");
    static_assert(std::is_same_v<typename DataFTraits::template Arg_t<2>, CountFRet>,
                  "Data function requires counting function return type as third argument");
    using T = std::remove_pointer_t<typename DataFTraits::template Arg_t<1>>;

    using RetType = std::optional<std::vector<T>>;
    std::vector<T> ret{};
    ret.resize(count_fcn(underlying));
    if (!ret.empty()) {
        const auto res = data_fcn(underlying, ret.data(), ret.size());
        if (res <= 0)
            return RetType{std::nullopt};
    }
    return RetType{ret};
}
template <typename U, typename CF, typename DF> auto wrap_oparm_owning_fill_freeable_arr(U underlying, CF count_fcn, DF data_fcn) {
    using CountFTraits = ext::function_traits<CF>;
    static_assert(CountFTraits::arity == 1, "Counting function requires one argument");
    static_assert(std::is_same_v<typename CountFTraits::template Arg_t<0>, U>, "Counting function requires the underlying ptr as argument");
    using CountFRet = typename CountFTraits::return_type;

    using DataFTraits = ext::function_traits<DF>;
    static_assert(DataFTraits::arity == 3, "Data function requires three arguments");
    static_assert(std::is_same_v<typename DataFTraits::template Arg_t<0>, U>, "Data function requires the underlying ptr as first argument");
    static_assert(std::is_pointer_v<typename DataFTraits::template Arg_t<0>>, "Data function requires a pointer to the array as second argument");
    static_assert(std::is_same_v<typename DataFTraits::template Arg_t<2>, CountFRet>,
                  "Data function requires counting function return type as third argument");
    using T = std::remove_pointer_t<typename DataFTraits::template Arg_t<1>>;

    using LocAlloc = NoallocWFree<T>;
    using RetType = std::optional<std::vector<gsl::owner<T>, LocAlloc>>;
    RetType ret{};
    auto& vec = ret.emplace();
    vec.resize(count_fcn(underlying));
    if (!vec.empty()) {
        const auto res = data_fcn(underlying, vec.data(), vec.size());
        if (res != 0)
            return RetType{std::nullopt};
    }
    return ret;
}

template <typename Conv = void, typename U, typename CF, typename DF, typename... DF_Args>
auto wrap_oparm_owning_fill_autodestroyable_arr(U underlying, CF count_fcn, DF data_fcn, DF_Args... df_args) {
    using CountFTraits = ext::function_traits<CF>;
    static_assert(CountFTraits::arity == 1, "Counting function requires one argument");
    static_assert(std::is_same_v<typename CountFTraits::template Arg_t<0>, U>, "Counting function requires the underlying ptr as argument");
    using CountFRet = typename CountFTraits::return_type;

    using DataFTraits = ext::function_traits<DF>;
    static_assert(DataFTraits::arity == 3 + sizeof...(DF_Args), "Data function requires three base arguments");
    static_assert(std::is_same_v<typename DataFTraits::template Arg_t<0>, U>, "Data function requires the underlying ptr as first argument");
    static_assert(std::is_pointer_v<typename DataFTraits::template Arg_t<0>>, "Data function requires a pointer to the array as second argument");
    static_assert(std::is_same_v<typename DataFTraits::template Arg_t<2>, CountFRet>,
                  "Data function requires counting function return type as third argument");
    using DedT = std::remove_pointer_t<typename DataFTraits::template Arg_t<1>>;
    if constexpr (!std::is_same_v<Conv, void>)
        static_assert(sizeof(DedT) == sizeof(Conv) && alignof(DedT) == alignof(Conv), "Conversion type must have the same size as the source");
    using T = std::conditional_t<std::is_same_v<Conv, void>, DedT, Conv>;

    using RetType = std::optional<std::vector<T>>;
    RetType ret{};
    auto& vec = ret.emplace();
    vec.resize(count_fcn(underlying));
    if (!vec.empty()) {
        const auto res = data_fcn(underlying, reinterpret_cast<DedT*>(vec.data()), vec.size(), df_args...); // C++2a: std::bit_cast
        if (res != 0)
            return RetType{std::nullopt};
    }
    return ret;
}

template <typename Wrap, template <class, class> typename Span = UniqueSpan, void (*dtroy)(Wrap*) = std::destroy_at<Wrap>, typename U,
          typename DataFRet, typename T, typename... DataFArgs>
decltype(auto) wrap_opram_owning_set_destroyable_arr(U underlying, DataFRet (*data_fcn)(U, T**, DataFArgs...), DataFArgs... data_f_args) {
    Wrap* lease_arr;
    auto res = data_fcn(underlying, reinterpret_cast<T**>(&lease_arr), data_f_args...);
    auto deleter = [=](Wrap* arr) {
        if (arr == nullptr)
            return res;

        for (auto it = arr; it != arr + res; ++it)
            dtroy(it);
        freeany(arr);
        return -1;
    };
    using RetType = Span<Wrap, std::conditional_t<std::is_same_v<Span<char, decltype(std::free)>, UniqueSpan<char, decltype(std::free)>>,
                                                  decltype(deleter), void (*)(Wrap*)>>;

    if constexpr (std::is_same_v<RetType, UniqueSpan<Wrap, decltype(deleter)>>) {
        return RetType{res != -1 ? lease_arr : nullptr, deleter};
    } else {
        if (res == -1)
            return RetType{nullptr, nullptr};
        return RetType{lease_arr, [](auto arr) {
                           auto it = arr;
                           while (it)
                               dtroy(it++);
                           freeany(arr);
                       }};
    }
}
} // namespace light
namespace heavy {
template <typename Conv = void, typename U, typename CountFRet, typename DataFRet, typename T>
auto wrap_oparm_owning_fill_static_arr(U underlying, CountFRet (*count_fcn)(U), DataFRet (*data_fcn)(U, T*, CountFRet)) {
    std::vector<gsl::owner<T>> ret{};
    ret.resize(count_fcn(underlying));
    const auto res = data_fcn(underlying, ret.data(), ret.size());
    if (res != 0)
        throw std::runtime_error{__func__};
    if constexpr (std::is_same_v<void, Conv>)
        return ret;
    std::vector<Conv> tret{};
    tret.reserve(ret.size());
    std::move(ret.begin(), ret.end(), std::back_inserter(tret));
    return tret;
}
template <typename Conv = void, typename U, typename CF, typename DF>
auto wrap_oparm_owning_fill_freeable_arr(U underlying, CF count_fcn, DF data_fcn) {
    using CountFTraits = ext::function_traits<CF>;
    static_assert(CountFTraits::arity == 1, "Counting function requires one argument");
    static_assert(std::is_same_v<typename CountFTraits::template Arg_t<0>, U>, "Counting function requires the underlying ptr as argument");
    using CountFRet = typename CountFTraits::return_type;

    using DataFTraits = ext::function_traits<DF>;
    static_assert(DataFTraits::arity == 3, "Data function requires three arguments");
    static_assert(std::is_same_v<typename DataFTraits::template Arg_t<0>, U>, "Data function requires the underlying ptr as first argument");
    static_assert(std::is_pointer_v<typename DataFTraits::template Arg_t<0>>, "Data function requires a pointer to the array as second argument");
    static_assert(std::is_same_v<typename DataFTraits::template Arg_t<2>, CountFRet>,
                  "Data function requires counting function return type as third argument");
    using T = std::remove_pointer_t<typename DataFTraits::template Arg_t<1>>;

    using LocAlloc = NoallocWFree<T>;
    std::vector<gsl::owner<T>, LocAlloc> ret{};
    ret.resize(count_fcn(underlying));
    const auto res = data_fcn(underlying, ret.data(), ret.size());
    if (res != 0)
        throw std::runtime_error{__func__};
    if constexpr (std::is_same_v<void, Conv>)
        return ret;
    std::vector<Conv> tret{};
    tret.reserve(ret.size());
    std::move(ret.begin(), ret.end(), std::back_inserter(tret));
    return tret;
}

template <typename Conv = void, typename U, typename CF, typename DF>
auto wrap_oparm_owning_fill_autodestroyable_arr(U underlying, CF count_fcn, DF data_fcn) {
    using CountFTraits = ext::function_traits<CF>;
    static_assert(CountFTraits::arity == 1, "Counting function requires one argument");
    static_assert(std::is_same_v<typename CountFTraits::template Arg_t<0>, U>, "Counting function requires the underlying ptr as argument");
    using CountFRet = typename CountFTraits::return_type;

    using DataFTraits = ext::function_traits<DF>;
    static_assert(DataFTraits::arity == 3, "Data function requires three arguments");
    static_assert(std::is_same_v<typename DataFTraits::template Arg_t<0>, U>, "Data function requires the underlying ptr as first argument");
    static_assert(std::is_pointer_v<typename DataFTraits::template Arg_t<0>>, "Data function requires a pointer to the array as second argument");
    static_assert(std::is_same_v<typename DataFTraits::template Arg_t<2>, CountFRet>,
                  "Data function requires counting function return type as third argument");
    using DedT = std::remove_pointer_t<typename DataFTraits::template Arg_t<1>>;
    using T = std::conditional_t<(sizeof(DedT) == sizeof(Conv) && alignof(DedT) == alignof(Conv)), Conv, DedT>;

    std::vector<T> ret{};
    ret.resize(count_fcn(underlying));
    const auto res = data_fcn(underlying, reinterpret_cast<DedT*>(ret.data()), ret.size()); // C++2a: std::bit_cast
    if (res != 0)
        throw std::runtime_error{__func__};
    if constexpr (std::is_same_v<void, Conv>)
        return ret;
    std::vector<Conv> tret{};
    tret.reserve(ret.size());
    std::move(ret.begin(), ret.end(), std::back_inserter(tret));
    return tret;
}

template <typename Wrap = void, typename U, typename DataFRet, typename T, typename... DataFArgs>
auto wrap_opram_owning_set_destroyable_arr(U underlying, DataFRet (*data_fcn)(U, T**, DataFArgs...), DataFArgs... data_f_args) {
    T* ptr;
    auto res = data_fcn(underlying, &ptr, data_f_args...);
    if (res == -1)
        throw std::runtime_error{__func__};
    using ValueType = std::conditional_t<std::is_void_v<Wrap>, T, Wrap>;
    std::vector<ValueType> ret;
    ret.reserve(res);
    auto it = ptr;
    const auto end = ptr + res;
    while (it != end)
        ret.emplace_back(*it++);
    freeany(ptr);
    return ret;
}
} // namespace heavy
} // namespace virt::meta

// From C++ Weekly - Ep 134 (Jason Turner) :
// https://www.youtube.com/watch?v=EsUmnLgz8QY

template <typename... Base> struct Visitor : Base... { using Base::operator()...; };

template <typename... T> Visitor(T...)->Visitor<T...>;