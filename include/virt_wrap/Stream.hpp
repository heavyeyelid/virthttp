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
    friend Connection;
    friend Domain;

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

    template <class T> using SinkFunc = int (*)(Stream st, const char* data, size_t nbytes, T* opaque);

    template <class T> using SinkHoleFunc = bool (*)(Stream st, long long length, T* opaque);

    template <class T> using SourceFunc = int (*)(Stream st, char* data, size_t nbytes, T* opaque);

    template <class T> using SourceHoleFunc = bool (*)(Stream st, int* inData, long long* length, T* opaque);

    template <class T> using SourceSkipFunc = bool (*)(Stream st, long long length, T* opaque);

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
    // template <class T> bool recvAll(SinkFunc<T> handler) noexcept;
    std::optional<long long> recvHole();
    int send(gsl::span<const char>) noexcept;
    int send(const char* buf, size_t buflen) noexcept;
    // template <class T> bool sendAll(SourceFunc<T> handler, T* opaque) noexcept;
    bool sendHole(long long) noexcept;
    // template <class T> bool sparseRecvAll(SinkFunc<T> handler, SinkHoleFunc<T> holeHandler, T* opaque) noexcept;
    // template <class T> int sparseSendAll(SourceFunc<T> handler, SourceHoleFunc<T> holeHandler,
    //                           SourceSkipFunc<T> skipHandler, T* opaque) noexcept;
};

[[nodiscard]] constexpr Stream::Flag operator|(Stream::Flag lhs, Stream::Flag rhs) noexcept;
constexpr Stream::Flag& operator|=(Stream::Flag& lhs, Stream::Flag rhs) noexcept;

}