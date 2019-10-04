#pragma once

#include <boost/beast.hpp>
#include "../general_store.hpp"
#include "http1/Session.hpp"
//#include "http2/Session.hpp"

// Accepts incoming connections and launches the sessions
class TcpListener : public std::enable_shared_from_this<TcpListener> {
    boost::beast::net::ip::tcp::acceptor acceptor_;
    boost::beast::net::ip::tcp::socket socket_;
    std::reference_wrapper<GeneralStore> gstore;

  public:
    TcpListener(boost::beast::net::io_context& ioc, const boost::beast::net::ip::tcp::endpoint& endpoint, GeneralStore& gstore)
        : acceptor_(ioc), socket_(ioc), gstore(gstore) {
        boost::beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if (ec) {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(boost::beast::net::socket_base::reuse_address(true), ec);
        if (ec) {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if (ec) {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(boost::beast::net::socket_base::max_listen_connections, ec);
        if (ec) {
            fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void run() {
        if (!acceptor_.is_open())
            return;
        do_accept();
    }

    void do_accept() { acceptor_.async_accept(socket_, std::bind(&TcpListener::on_accept, shared_from_this(), std::placeholders::_1)); }

    void on_accept(boost::beast::error_code ec) {
        if (ec)
            fail(ec, "accept");
        else {
            // Create the Session and run it
            /// FIXME: select the right kind of session to create
            std::make_shared<Session>(std::move(socket_), gstore)->run();
        }

        // Accept another connection
        do_accept();
    }
};