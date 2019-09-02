#pragma once

#include <array>
#include <libvirt/libvirt.h>
#include "fwd.hpp"
#include "utils.hpp"

namespace virt {

class Stream {
    virStreamPtr underlying{};
    constexpr explicit Stream(virStreamPtr ptr) noexcept;

  public:
    enum class Flag : unsigned {
        NONBLOCK = VIR_STREAM_NONBLOCK,
    };
    class FlagsC : public EnumSetHelper<FlagsC, Flag> {
        using Base = EnumSetHelper<FlagsC, Flag>;
        friend Base;
        constexpr static std::array values = {"nonblock"};
    } constexpr static Flags{};
    enum class RecvFlag {
        STOP_AT_HOLE = VIR_STREAM_RECV_STOP_AT_HOLE,
    };
    class RecvFlagsC : public EnumSetHelper<RecvFlagsC, RecvFlag> {
        using Base = EnumSetHelper<RecvFlagsC, RecvFlag>;
        friend Base;
        constexpr static std::array values = {"stop_at_hole"};
    } constexpr static RecvFlagsValues{};

    Stream(const Connection&, Flag) noexcept;
    Stream(const Stream&) noexcept;
    constexpr Stream(Stream&&) noexcept;
    ~Stream() noexcept;
    Stream& operator=(const Stream&) noexcept;
    constexpr Stream& operator=(Stream&&) noexcept;

    bool abort() noexcept;
    bool finish() noexcept;
    int recv(gsl::span<char>) noexcept;
    int recv(char* buf, size_t buflen) noexcept;
    int recv(gsl::span<char>, RecvFlag) noexcept;
    int recv(char* buf, size_t buflen, RecvFlag) noexcept;
    std::optional<long long> recvHole();
    int send(gsl::span<const char>) noexcept;
    int send(const char* buf, size_t buflen) noexcept;
};

[[nodiscard]] constexpr Stream::Flag operator|(Stream::Flag lhs, Stream::Flag rhs) noexcept;
constexpr Stream::Flag& operator|=(Stream::Flag& lhs, Stream::Flag rhs) noexcept;

}