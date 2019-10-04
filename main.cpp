//
// Created by hugo on 30.01.19.
//
#include <iostream>
#include <gsl/gsl>
#include "wrapper/config.hpp"
#include "wrapper/general_store.hpp"
#include "wrapper/http_wrapper.hpp"
#include "logger.hpp"

using namespace std::literals;

int main(int argc, char** argv) {
    GeneralStore gstore{IniConfig{"config.ini"}};

    logger.info("libvirt server URI: ", gstore.config().getConnURI());
    logger.info("http server URI: ", gstore.config().getHttpURI());
    if (!gstore.config().isHTTPAuthRequired())
        logger.warning("The HTTP authentication is disabled! Beware of unauthorized access!");

    const auto address = boost::beast::net::ip::make_address(gstore.config().http_address);
    const auto port = static_cast<unsigned short>(gstore.config().http_port);
    const auto doc_root = std::make_shared<std::string>(gstore.config().http_doc_root);
    const auto threads = std::max(1, gsl::narrow_cast<int>(gstore.config().http_threads));

    // The io_context is required for all I/O
    boost::beast::net::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<TcpListener>(ioc, boost::beast::net::ip::tcp::endpoint{address, port}, gstore)->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back([&ioc] { ioc.run(); });
    ioc.run();

    // iniConfig.reset(nullptr); // Better to run destructors in the lifetime of main
    return EXIT_SUCCESS;
}