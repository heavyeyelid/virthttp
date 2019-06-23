#pragma once

// True False Error
class TFE {
    enum class Impl { t = 2, f = 1, e = 0 };
    Impl v;

  public:
    constexpr explicit TFE(int tfe) : v(Impl{tfe + 1}) {}
    constexpr explicit operator bool() const noexcept { return v == Impl::t; }
    constexpr bool err() const noexcept { return v == Impl::e; }
};