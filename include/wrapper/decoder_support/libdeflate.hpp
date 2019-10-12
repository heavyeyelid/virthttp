#pragma once
#include <memory>
#include <string>
#include <libdeflate.h>

namespace libdeflate {

constexpr auto compression_level = 11;

enum class Mode { raw_deflate, zlib, gzip };

bool compress(std::string& body, Mode mode) noexcept {
    std::unique_ptr<struct libdeflate_compressor, void (*)(struct libdeflate_compressor*)> c = {libdeflate_alloc_compressor(compression_level),
                                                                                                &libdeflate_free_compressor};
    if (!c)
        return false;

    std::string out;
    out.resize(body.size());

    const auto actual_compressed_size = [&]() {
        switch (mode) {
        case Mode::raw_deflate:
            return libdeflate_deflate_compress(c.get(), body.data(), body.size(), out.data(),
                                               libdeflate_deflate_compress_bound(c.get(), body.size()));
        case Mode::zlib:
            return libdeflate_zlib_compress(c.get(), body.data(), body.size(), out.data(), libdeflate_zlib_compress_bound(c.get(), body.size()));
        case Mode::gzip:
            return libdeflate_gzip_compress(c.get(), body.data(), body.size(), out.data(), libdeflate_gzip_compress_bound(c.get(), body.size()));
        }
    }();
    if (actual_compressed_size == 0)
        return false;

    out.resize(actual_compressed_size);
    body = std::move(out);
    return true;
}

}