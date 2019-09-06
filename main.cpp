//
// Created by hugo on 30.01.19.
//
#include <iostream>
#include <gsl/gsl>
#include <libvirt/libvirt.h>
#include "wrapper/config.hpp"
#include "wrapper/http_wrapper.hpp"
#include "fwd.hpp"
#include "logger.hpp"
#include "virt_wrap.hpp"

using namespace std::literals;

int main(int argc, char** argv) {
    iniConfig.init("config.ini");
    logger.info("libvirt server URI: ", iniConfig.getConnURI());
    logger.info("http server URI: ", iniConfig.getHttpURI());
    if (!iniConfig.isHTTPAuthRequired())
        logger.warning("The HTTP authentication is disabled! Beware of unauthorized access!");

    const auto address = net::ip::make_address(iniConfig.http_address);
    const auto port = static_cast<unsigned short>(iniConfig.http_port);
    const auto doc_root = std::make_shared<std::string>(iniConfig.http_doc_root);
    const auto threads = std::max(1, gsl::narrow_cast<int>(iniConfig.http_threads));

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<Listener>(ioc, tcp::endpoint{address, port}, doc_root)->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back([&ioc] { ioc.run(); });
    ioc.run();

    // iniConfig.reset(nullptr); // Better to run destructors in the lifetime of
    // main
    return EXIT_SUCCESS;
}

#include "virt_wrap/Connection.hpp"
#include "virt_wrap/Domain.hpp"
#include "virt_wrap/Network.hpp"
#include "virt_wrap/TypesParam.hpp"
#include "virt_wrap/impl/Connection.hpp"
#include "virt_wrap/impl/Domain.hpp"
#include "virt_wrap/impl/Network.hpp"
#include "virt_wrap/impl/NodeDevice.hpp"
#include "virt_wrap/impl/Stream.hpp"
#include "virt_wrap/impl/TypedParams.hpp"