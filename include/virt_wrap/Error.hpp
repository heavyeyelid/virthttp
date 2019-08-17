#pragma once
#include <string>
#include <gsl/gsl>
#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>

namespace virt {
class Error;
class ErrorRef;

ErrorRef getLastError() noexcept;
Error extractLastError();

class ErrorRef {
    virErrorPtr underlying;
    constexpr explicit ErrorRef(virErrorPtr&& u) : underlying(u) {}
    friend ErrorRef getLastError() noexcept;
    friend Error extractLastError();

  public:
    inline ~ErrorRef() noexcept { virFreeError(underlying); }
    [[nodiscard]] constexpr gsl::czstring<> message() const noexcept { return underlying->message; };
};

class Error {
    using Code = virErrorNumber;
    using Level = virErrorLevel;

  public:
    Code code;
    Level level;
    std::string message;

  private:
    inline explicit Error(virErrorPtr p) : code(Code((p ? p->code : 0))), level(p ? p->level : Level(0)), message(p ? p->message : nullptr) {}
    friend Error extractLastError();

  public:
    inline explicit operator bool() const noexcept { return message.c_str() != nullptr; }
};

inline ErrorRef getLastError() noexcept { return ErrorRef{virGetLastError()}; }
inline Error extractLastError() {
    const auto e = Error{virGetLastError()};
    virResetLastError();
    return e;
}
}