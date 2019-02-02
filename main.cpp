//
// Created by hugo on 30.01.19.
//
#include <libvirt/libvirt.h>
#include <iostream>
#include <INIReader.h>
#include "include/wrapper/http_wrapper.hpp"
#include "include/logger.hpp"

std::string config_file_loc = "config.ini";

int main(int argc, char* argv[])
{
    INIReader reader(config_file_loc);

    if (reader.ParseError() < 0) {
        warning("Can't load " + config_file_loc);
        warning("Using default config");
    }

    auto const address = net::ip::make_address(reader.Get("http_server","address","0.0.0.0"));
    auto const port = static_cast<unsigned short>(std::atoi(reader.Get("http_server", "port", "8081").c_str()));
    auto const doc_root = std::make_shared<std::string>(reader.Get("http_server", "doc_root", "."));
    auto const threads = std::max<int>(1, std::atoi(reader.Get("http_server", "threads", "1").c_str()));

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<listener>(
            ioc,
            tcp::endpoint{address, port}, doc_root)->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
                [&ioc]
                {
                    ioc.run();
                });
    ioc.run();

    return EXIT_SUCCESS;
}