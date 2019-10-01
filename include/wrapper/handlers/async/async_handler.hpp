#pragma once
#include <string_view>
#include <boost/beast/http.hpp>
#include <ctre.hpp>
#include "wrapper/general_store.hpp"
#include "wrapper/handlers/proto.hpp"

using namespace std::literals;

constexpr static ctll::fixed_string numeric_id_pattern = "[0-9A-Fa-f]{1,8}";
constexpr auto numeric_id_match(std::string_view id) noexcept { return ctre::match<numeric_id_pattern>(id); }

// Contract: sv matches `numeric_id_pattern`
constexpr std::uint32_t hex_decode_id(std::string_view sv) noexcept {
    std::uint32_t ret = 0;
    for (char c : sv) {
        ret <<= 4u;

        if (c >= '0' && '9' >= c)
            ret += c - '0';
        else if (c >= 'A' && 'F' >= c)
            ret += c - 'A' + 10;
        else if (c >= 'a' && 'f' >= c)
            ret += c - 'a' + 10;
    }
    return ret;
}
constexpr std::array<char, sizeof(std::uint32_t) * 2> hex_encode_id(std::uint32_t id) noexcept {
    constexpr auto hex = "0123456789abcdef"sv;
    std::array<char, sizeof(std::uint32_t)* 2> ret = {};
    for (auto it = ret.rbegin(); it < ret.rend(); ++it) {
        *it = hex[id & 0xfu];
        id >>= 16u;
    }
    return ret;
}

template <TransportProto proto> auto handle_async_retrieve(GeneralStore& gstore, std::string_view str_id) {
    if constexpr (proto == TransportProto::HTTP1) {
        using Ret = std::pair<boost::beast::http::status, std::string>;

        if (!numeric_id_match(str_id))
            return Ret{boost::beast::http::status::not_found, {}};

        auto [status, val] = gstore.async_store.value_if_ready(hex_decode_id(str_id));
        switch (status) {
        case AsyncStore::TaskStatus::non_existent:
            return Ret{boost::beast::http::status::not_found, {}};
        case AsyncStore::TaskStatus::in_progress:
            return Ret{boost::beast::http::status::processing, {}};
        case AsyncStore::TaskStatus::finished:
            return Ret{boost::beast::http::status::found, val};
        }
    }
}