#pragma once
#include <charconv>
#include <cstring>
#include <string>
#include <boost/beast.hpp>
#include <ctre.hpp>
#include <flatmap.hpp>
#include <zlib.h>
#include "zlib_beast_impl.hpp"

inline bool inflate_zlib(std::string& body, int wbits);
inline bool deflate_zlib(std::string& body, int wbits);

enum class Algs {
    // compress, // unsupported
    deflate,
    gzip,
    identity,
    // br, // unsupported
};

bool handle_decompression(const boost::beast::http::basic_fields<std::allocator<char>>& headers, std::string& body) {

    const auto head = headers[boost::beast::http::field::content_encoding];
    if (head.empty())
        return true;
    if (head == "identity")
        return true;
    if (head == "deflate")
        return inflate_zlib(body, 0);
    /* // We assume this doesn't work since the gzip compression doesn't work either
    if (head == "gzip")
        return inflate_zlib(body, 16);
        */
    return false;
}

constexpr static ctll::fixed_string weigthed_encodings_pattern = "([a-z]+|\\*)(?:;q=([0-1].?(?:[\\d]+)?)?)?(?:,\\s*)?";

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
    if (const auto it = accepted_values.find("deflate"sv); it != accepted_values.end()) {
        out_head.set(boost::beast::http::field::content_encoding, "deflate");
        return deflate_zlib(body, 15);
    }
    /* // We need to investigate that later on
    if (const auto it = accepted_values.find("gzip"sv); it != accepted_values.end()){
        out_head.set(boost::beast::http::field::content_encoding, "gzip");
        return deflate_zlib(body, 16 + 11);
    }
     */

    return true;
}

inline bool inflate_zlib(std::string& body, int wbits) {
    boost::beast::zlib::z_params zs;
    std::string out;
    std::memset(&zs, 0, sizeof(zs));
    zs.next_in = body.data();
    zs.avail_in = body.size();
    out.resize(body.size());
    zs.next_out = &out[0];
    zs.avail_out = out.size();

    InflateStream is;
    is.reset(wbits);
    for (;;) {
        boost::system::error_code ec;
        is.write(zs, boost::beast::zlib::Flush::sync, ec);
        if (ec)
            return false;
        if (zs.avail_out > 0)
            break;
        out.resize(2 * zs.total_out);
        zs.next_out = &out[zs.total_out];
        zs.avail_out = out.size() - zs.total_out;
    }
    out.resize(zs.total_out);
    body = std::move(out);
    return true;
}

inline bool deflate_zlib(std::string& body, int wbits) {
    boost::beast::zlib::z_params zs;
    std::memset(&zs, 0, sizeof(zs));
    DeflateStream ds;
    ds.reset(Z_DEFAULT_COMPRESSION, wbits, 4, boost::beast::zlib::Strategy::normal);
    std::string out;
    out.resize(deflate_upper_bound(body.size()));
    zs.next_in = body.data();
    zs.avail_in = body.size();
    zs.next_out = out.data();
    zs.avail_out = out.size();
    boost::system::error_code ec;
    ds.write(zs, boost::beast::zlib::Flush::full, ec);
    if (ec)
        return false;
    out.resize(zs.total_out);
    body = std::move(out);
    return true;
}