//
// Created by hugo on 30.01.19.
//
extern "C" {
#include <libvirt/libvirt.h>
}

#include <iostream>
#include "mainClass.hpp"
#include "listener.hpp"

using Resource = UpperClass;

int main(int argc, char *argv[])
{
    virConnectPtr conn;

    conn = virConnectOpen("qemu:///system");
    if (conn == nullptr) {
        std::cout << "Failed to open connection to qemu:///system !\n";
        return 1;
    } else {
        std::cout << "Connection to qemu:///system established !\n";
    }
/*
    if (argc != 2 && argc != 3) {
        std::cerr <<
                  "Usage: http-server-flex <port> [threads]\n"
                  "Example:\n"
                  "    http-server-flex 0.0.0.0 8080 .\n";
        return EXIT_FAILURE;
    }
    const auto address = boost::asio::ip::make_address("0.0.0.0");
    const auto port = static_cast<unsigned short>(std::atoi(argv[1]));
    const auto threads = std::max<int>(1, (argc == 3 ? std::atoi(argv[2]) : 1));

    auto data = std::make_shared<Resource>();

    // The io_context is required for all I/O
    boost::asio::io_context ioc{threads};


    // Create and launch a listening port
    std::make_shared<Listener>(
            ioc,
            tcp::endpoint{address, port},
            data)->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
                [&ioc] {
                    ioc.run();
                });
    ioc.run();*/

    virConnectClose(conn);
    return 0;
}