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
    inline ~ErrorRef() noexcept { virFreeError(underlying); }
    friend ErrorRef getLastError() noexcept;
    friend Error extractLastError();

  public:
    [[nodiscard]] constexpr gsl::czstring<> message() const noexcept { return underlying->message; };
};

class Error {
    using Code = virErrorNumber;
    using Level = virErrorLevel;
    Code code;
    Level level;
    std::string message;

    inline explicit Error(virErrorPtr p) : code(Code((p ? p->code : 0))), level(p ? p->level : Level(0)), message(p ? p->message : nullptr) {}
    inline explicit operator bool() const noexcept { return message.c_str() != nullptr; }
    friend Error extractLastError();
};

inline ErrorRef getLastError() noexcept { return ErrorRef{virGetLastError()}; }
inline Error extractLastError() {
    const auto e = virGetLastError();
    virResetLastError();
    return Error{e};
}
}