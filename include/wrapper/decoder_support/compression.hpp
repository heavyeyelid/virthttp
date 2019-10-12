#pragma once
#include <charconv>
#include <cstring>
#include <string>
#include <boost/beast.hpp>
#include <ctre.hpp>
#include <flatmap.hpp>
#include "libdeflate.hpp"

inline bool inflate_zlib(std::string& body, int wbits);

enum class Algs {
    // compress, // unsupported
    deflate,
    gzip,
    identity,
    // br, // unsupported
};

constexpr static ctll::fixed_string weigthed_encodings_pattern = R"(([a-z]+|\*)(?:;q=([0-1].?(?:[\d]+)?)?)?(?:,\s*)?)";

bool handle_compression(const boost::beast::http::basic_fields<std::allocator<char>>& in_head,
                        boost::beast::http::basic_fields<std::allocator<char>>& out_head, std::string& body) {
    /// TODO check if the content-type looks like it could be compressed
    /// TODO check if the body is large enough to have any advantage compressing it
    const auto in_algs = in_head[boost::beast::http::field::accept_encoding];
    if (in_algs.empty())
        return true;
    flatmap<std::string_view, float> accepted_values;
    for (auto [full, name, weight] : ctre::range<weigthed_encodings_pattern>(in_algs)) {
        float w = 1;
        if (weight) {
            const std::string w_str = {weight.to_view().data(), weight.to_view().size()};
            w = std::atof(w_str.c_str());
            // at time of writing, only MSVC has a std::from_chars implementation for floating points
            // std::from_chars(weight.begin(), weight.end(), w);
        }
        accepted_values[name.to_view()] = w;
    }

    /*
     * https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Accept-Encoding:
     * As long as the identity value, meaning no encoding, is not explicitly forbidden, by an identity;q=0 or a *;q=0 without another explicitly set
     * value for identity, the server must never send back a 406 Not Acceptable error.
     * */
    if (const auto it = accepted_values.find("identity"sv); it != accepted_values.end() && it->second == 0.0f)
        return false;

    // Hack; waiting for C++2a
    if (const auto it = accepted_values.find("gzip"sv); it != accepted_values.end()) {
        const auto res = libdeflate::compress(body, libdeflate::Mode::gzip);
        if (res)
            out_head.set(boost::beast::http::field::content_encoding, "gzip");
        return res;
    }
    if (const auto it = accepted_values.find("deflate"sv); it != accepted_values.end()) {
        const auto res = libdeflate::compress(body, libdeflate::Mode::zlib);
        if (res)
            out_head.set(boost::beast::http::field::content_encoding, "deflate");
        return res;
    }

    return true;
}
