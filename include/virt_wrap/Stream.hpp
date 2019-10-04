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

    class Flag;
    class RecvFlag;

    template <class T> using SinkFunc = int (*)(Stream st, const char* data, size_t nbytes, T* opaque);

    template <class T> using SinkHoleFunc = bool (*)(Stream st, long long length, T* opaque);

    template <class T> using SourceFunc = int (*)(Stream st, char* data, size_t nbytes, T* opaque);

    template <class T> using SourceHoleFunc = bool (*)(Stream st, int* inData, long long* length, T* opaque);

    template <class T> using SourceSkipFunc = bool (*)(Stream st, long long length, T* opaque);

    inline Stream(const Connection&, Flag) noexcept;
    inline Stream(const Stream&) noexcept;
    constexpr Stream(Stream&&) noexcept;
    inline ~Stream() noexcept;
    inline Stream& operator=(const Stream&) noexcept;
    constexpr Stream& operator=(Stream&&) noexcept;

    inline bool abort() noexcept;
    inline bool finish() noexcept;
    inline int recv(gsl::span<char>) noexcept;
    inline int recv(char* buf, size_t buflen) noexcept;
    inline int recv(gsl::span<char>, RecvFlag) noexcept;
    inline int recv(char* buf, size_t buflen, RecvFlag) noexcept;
    // template <class T> bool recvAll(SinkFunc<T> handler) noexcept;
    inline std::optional<long long> recvHole();
    inline int send(gsl::span<const char>) noexcept;
    inline int send(const char* buf, size_t buflen) noexcept;
    // template <class T> bool sendAll(SourceFunc<T> handler, T* opaque) noexcept;
    inline bool sendHole(long long) noexcept;
    // template <class T> bool sparseRecvAll(SinkFunc<T> handler, SinkHoleFunc<T> holeHandler, T* opaque) noexcept;
    // template <class T> int sparseSendAll(SourceFunc<T> handler, SourceHoleFunc<T> holeHandler,
    //                           SourceSkipFunc<T> skipHandler, T* opaque) noexcept;
};

}
#include "enums/Stream/Stream.hpp"
#include "impl/Stream.hpp"