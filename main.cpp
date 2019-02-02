//
// Created by hugo on 30.01.19.
//
#include <iostream>
#include <INIReader.h>
#include <gsl/gsl>
#include <libvirt/libvirt.h>
#include "wrapper/http_wrapper.hpp"
#include "wrapper/config.hpp"
#include "fwd.hpp"
#include "logger.hpp"
#include "virt_wrap.hpp"

using namespace std::literals;

int main(int argc, const gsl::czstring<> *const argv){
    iniConfig = std::make_unique<IniConfig>(config_file_loc);
    logger.setQuiet(false);
    logger.setDebug(true);

    const INIReader reader(config_file_loc);

    if (reader.ParseError() < 0) {
        logger.warning("Can't load ", config_file_loc);
        logger.info("Using default config");
    }

    logger.info("libvirt server URI: ", iniConfig->getConnURI());

    const auto address = net::ip::make_address(reader.Get("http_server","address","0.0.0.0"));
    const auto port = static_cast<unsigned short>(reader.GetInteger("http_server", "port", 8081));
    const auto doc_root = std::make_shared<std::string>(reader.Get("http_server", "doc_root", "."));
    const auto threads = std::max(1, gsl::narrow_cast<int>(reader.GetInteger("http_server", "threads", 1)));

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<Listener>(
            ioc,
            tcp::endpoint{address, port}, doc_root)->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
                [&ioc]{
                    ioc.run();
                });
    ioc.run();

    iniConfig.reset(nullptr); // Better to run destructors in the lifetime of main
    return EXIT_SUCCESS;
}

#include "wrapper/impl/config.hpp"