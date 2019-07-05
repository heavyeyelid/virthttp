//
// Created by _as on 2019-01-31.
//

#pragma once

#include <algorithm>
#include <cstdlib>
#include <type_traits>

template <typename T> using passive = T;

template <typename T> inline void freeany(T ptr) {
    static_assert(std::is_array_v<T> || std::is_pointer_v<T>, "T needs to be a pointer or an array type");
    std::free(ptr);
}

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
    inline const auto cbegin() const noexcept { return this->get(); }
    inline const auto cend() const noexcept { return false_it(this->get()); }
};

namespace virt::meta {
namespace impl::any {
// Need template lambdas to reduce bloat, as types need to be passed around
}
namespace light {
template <typename U, typename CountFRet, typename DataFRet, typename T>
auto wrap_oparm_owning_fill_static_arr(U underlying, CountFRet (*count_fcn)(U), DataFRet (*data_fcn)(U, T*, CountFRet)) {
    using RetType = std::optional<std::vector<T>>;
    std::vector<T> ret{};
    ret.resize(count_fcn(underlying));
    if (!ret.empty()) {
        const auto res = data_fcn(underlying, ret.data(), ret.size());
        if (res != 0)
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
    using RetType = std::optional<std::vector<T, LocAlloc>>;
    std::vector<gsl::owner<T>, LocAlloc> ret{};
    ret.resize(count_fcn(underlying));
    if (!ret.empty()) {
        const auto res = data_fcn(underlying, ret.data(), ret.size());
        if (res != 0)
            return RetType{std::nullopt};
    }
    return RetType{ret};
}

template <typename Wrap, typename U, typename DataFRet, typename T, typename... DataFArgs>
auto wrap_opram_owning_set_autodestroyable_arr(U underlying, DataFRet (*data_fcn)(U, T**, DataFArgs...), DataFArgs... data_f_args)
    -> UniqueFalseTerminatedSpan<Wrap, void (*)(Wrap*)> { // This one can shadow
    using RetType = UniqueFalseTerminatedSpan<Wrap, void (*)(Wrap*)>;
    Wrap* lease_arr;
    auto res = data_fcn(underlying, reinterpret_cast<T**>(&lease_arr), data_f_args...);
    if (res == -1)
        return {nullptr, nullptr};
    return {lease_arr, [](auto arr) {
                auto it = arr;
                while (it++)
                    it->~Wrap();
                freeany(arr);
            }};
}

template <typename Wrap, void (*dtroy)(Wrap*) = std::destroy_at<Wrap>, typename U, typename DataFRet, typename T, typename... DataFArgs>
auto wrap_opram_owning_set_destroyable_arr(U underlying, DataFRet (*data_fcn)(U, T**, DataFArgs...), DataFArgs... data_f_args) {
    if constexpr (dtroy == std::destroy_at<Wrap>) {
        return wrap_opram_owning_set_autodestroyable_arr<Wrap>(underlying, data_fcn, data_f_args...);
    }
    using RetType = UniqueFalseTerminatedSpan<Wrap, void (*)(Wrap*)>;
    Wrap* lease_arr;
    auto res = data_fcn(underlying, reinterpret_cast<T**>(&lease_arr), data_f_args...);
    if (res == -1)
        return RetType{nullptr, nullptr};
    return RetType(lease_arr, [](Wrap* arr) {
        auto it = arr;
        while (it++)
            dtroy(it);
        freeany(arr);
    });
}
} // namespace light
namespace heavy {
template <typename Conv = void, typename U, typename CountFRet, typename DataFRet, typename T>
auto wrap_oparm_owning_fill_static_arr(U underlying, CountFRet (*count_fcn)(U), DataFRet (*data_fcn)(U, T*, CountFRet)) {
    std::vector<gsl::owner<T>> ret{};
    ret.resize(count_fcn(underlying));
    if (!ret.empty()) {
        const auto res = data_fcn(underlying, ret.data(), ret.size());
        if (res != 0)
            throw std::runtime_error{__func__};
    }
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
    if (!ret.empty()) {
        const auto res = data_fcn(underlying, ret.data(), ret.size());
        if (res != 0)
            throw std::runtime_error{__func__};
    }
    if constexpr (std::is_same_v<void, Conv>)
        return ret;
    std::vector<Conv> tret{};
    tret.reserve(ret.size());
    std::move(ret.begin(), ret.end(), std::back_inserter(tret));
    return tret;
}

template <typename Wrap, typename U, typename DataFRet, typename T, typename... DataFArgs>
auto wrap_opram_owning_set_destroyable_arr(U underlying, DataFRet (*data_fcn)(U, T**, DataFArgs...), DataFArgs... data_f_args) {
    T* ptr;
    auto res = data_fcn(underlying, &ptr, data_f_args...);
    if (res == -1)
        throw std::runtime_error{__func__};
    std::vector<Wrap> ret;
    ret.reserve(res);
    auto it = ptr;
    while (*it)
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