#pragma once
#include <cstddef>
#include "fwd.hpp"

namespace virt {
struct CpuMap {
    unsigned char* underlying{};
    int maplen{};

    constexpr CpuMap() noexcept = default;
    constexpr CpuMap(unsigned char* p, int l) noexcept : underlying(p), maplen(l) {}
    constexpr CpuMap(const CpuMap&) noexcept = default;
    constexpr CpuMap(CpuMap&&) noexcept = default;
    ~CpuMap() noexcept = default;
};
}