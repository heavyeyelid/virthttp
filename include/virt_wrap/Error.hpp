#pragma once
#include <string>
#include <gsl/gsl>
#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>

namespace virt {
struct Error;
class ErrorRef;

auto getLastError() noexcept -> ErrorRef;
auto extractLastError() -> Error;

class ErrorRef {
    virErrorPtr underlying;
    constexpr explicit ErrorRef(virErrorPtr&& u) : underlying(u) {}
    friend auto getLastError() noexcept -> ErrorRef;
    friend auto extractLastError() -> Error;

  public:
    inline ~ErrorRef() noexcept { virFreeError(underlying); }
    [[nodiscard]] constexpr auto message() const noexcept -> gsl::czstring<> { return underlying->message; };
};

struct Error {
    using Code = virErrorNumber;
    using Level = virErrorLevel;

  public:
    Code code;
    Level level;
    std::string message;

  private:
    inline explicit Error(virErrorPtr p)
        : code(Code((p ? p->code : 0))), level(p ? p->level : Level(0)), message(p ? std::string{p->message} : std::string{}) {}
    friend auto extractLastError() -> Error;

  public:
    inline explicit operator bool() const noexcept { return !message.empty(); }
};

inline auto getLastError() noexcept -> ErrorRef { return ErrorRef{virGetLastError()}; }
inline auto extractLastError() -> Error {
    const auto e = Error{virGetLastError()};
    virResetLastError();
    return e;
}
} // namespace virt