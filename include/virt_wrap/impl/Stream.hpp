#pragma once
#include "../Stream.hpp"
#include "Connection.hpp"

namespace virt {
constexpr Stream::Stream(virStreamPtr ptr) noexcept : underlying(ptr) {}
Stream::Stream(const Connection& conn, Flag flags) noexcept : underlying(virStreamNew(conn.underlying, to_integral(flags))) {}
Stream::Stream(const Stream& oth) noexcept : underlying(oth.underlying) {
    if (underlying)
        virStreamRef(underlying);
}
constexpr Stream::Stream(Stream&& from) noexcept : underlying(from.underlying) { from.underlying = nullptr; }
Stream::~Stream() noexcept {
    if (underlying)
        virStreamFree(underlying);
}
Stream& Stream::operator=(const Stream& oth) noexcept {
    this->~Stream();
    if ((underlying = oth.underlying))
        virStreamRef(underlying);
    return *this;
}
constexpr Stream& Stream::operator=(Stream&& from) noexcept {
    underlying = from.underlying;
    from.underlying = nullptr;
    return *this;
}

inline bool Stream::abort() noexcept { return virStreamAbort(underlying) >= 0; }

bool Stream::finish() noexcept { return virStreamFinish(underlying) >= 0; }
int Stream::recv(gsl::span<char> span) noexcept { return recv(span.data(), span.size()); }
int Stream::recv(char* buf, size_t buflen) noexcept { return virStreamRecv(underlying, buf, buflen); }
int Stream::recv(gsl::span<char> span, RecvFlag flags) noexcept { return recv(span.data(), span.size(), flags); }
int Stream::recv(char* buf, size_t buflen, RecvFlag flags) noexcept { return virStreamRecvFlags(underlying, buf, buflen, to_integral(flags)); }
std::optional<long long> Stream::recvHole() {
    std::optional<long long> ret;
    long long& v = ret.emplace();
    return virStreamRecvHole(underlying, &v, 0) >= 0 ? ret : std::nullopt;
}
int Stream::send(gsl::span<const char> span) noexcept { return send(span.data(), span.size()); }
int Stream::send(const char* buf, size_t buflen) noexcept { return virStreamSend(underlying, buf, buflen); }

[[nodiscard]] constexpr Stream::Flag operator|(Stream::Flag lhs, Stream::Flag rhs) noexcept {
    return Stream::Flag{to_integral(lhs) | to_integral(rhs)};
}
constexpr Stream::Flag& operator|=(Stream::Flag& lhs, Stream::Flag rhs) noexcept { return lhs = Stream::Flag{to_integral(lhs) | to_integral(rhs)}; }
}