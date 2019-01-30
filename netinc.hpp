//
// Created by hugo on 30.01.19.
//
#pragma once
#include <boost/asio/ts/net.hpp>

namespace net {
    using io_context = boost::asio::io_context;
    template <typename Executor>
    using strand = boost::asio::strand<Executor>;
    using socket_base = boost::asio::socket_base;
    namespace ip {
        using tcp = boost::asio::ip::tcp;
        using ucp = boost::asio::ip::udp;
        using address = boost::asio::ip::address;
        address (& make_address)(const char*) = boost::asio::ip::make_address;
    }
    using boost::asio::bind_executor;
}