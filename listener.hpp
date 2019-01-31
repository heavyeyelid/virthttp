//
// Created by hugo on 30.01.19.
//
#include <boost/beast.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <gsl/gsl>
#include <rapidjson/rapidjson.h>

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>
namespace json = rapidjson;

struct UpperClass {

};

using Resource = UpperClass;

// from https://stackoverflow.com/a/14530993/7486697
std::string url_decode(boost::beast::string_view src) {
    std::string ret;
    ret.reserve(src.size());
    char a, b;
    auto it = src.begin();
    while (it != src.end()) {
        if ((*it == '%') && ((a = *(it + 1)) && (b = *(it + 2))) && (std::isxdigit(a) && std::isxdigit(b))) {
            if (a >= 'a')
                a -= 'a' - 'A';
            if (a >= 'A')
                a -= ('A' - 10);
            else
                a -= '0';
            if (b >= 'a')
                b -= 'a' - 'A';
            if (b >= 'A')
                b -= ('A' - 10);
            else
                b -= '0';
            ret.push_back(gsl::narrow_cast<char>(16 * a + b));
            it += 3;
        } else if (*it == '+') {
            ret.push_back(' ');
            ++it;
        } else {
            ret.push_back(*it++);
        }
    }
    return ret;
}


template<class Body, class Allocator, class Send>
void handle_request(const Resource &data,
                    http::request<Body, http::basic_fields<Allocator>> &&req,
                    Send &&send) {
    const auto invalid = [&](std::string_view why) {
        boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::bad_request,
                                                                          req.version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = why;
        res.prepare_payload();
        return res;
    };

    std::cout << "Recieved from a session: HTTP " << req.method() << ' ' << req.target() << std::endl;

    // Make sure we can handle the method
    if (req.method() != boost::beast::http::verb::get && req.method() != boost::beast::http::verb::head)
        return send(invalid("Unknown HTTP-method"));

    if (req.target().empty() || req.target()[0] != '/') {
        return send(invalid("Empty request"));
    }

    if (req.target().starts_with("/zipinfo/")) {
        const auto zip_str = req.target().substr(9);

        if (zip_str.size() != 4 ||
            std::any_of(zip_str.cbegin(), zip_str.cend(), [](auto c) { return !std::isdigit(c); }))
            return send(invalid("Invalid PLZ"));


    } else
        return send(invalid("No such service"));

    // Respond to HEAD request
    if (req.method() == boost::beast::http::verb::head) {
        boost::beast::http::response<boost::beast::http::empty_body> res{boost::beast::http::status::ok, req.version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }

    // Respond to GET request
    boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::ok, req.version()};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());
    return send(std::move(res));
}

//------------------------------------------------------------------------------

// Handles an HTTP server connection.
// This uses the Curiously Recurring Template Pattern so that
// the same code works with both SSL streams and regular sockets.
template<class Derived>
class ServerSession {
    // Access the derived class, this is part of
    // the Curiously Recurring Template Pattern idiom.
    Derived &derived() {
        return static_cast<Derived &>(*this);
    }

    // Use a templated lambda (or conceptually generic) once in C++2a
    struct Send {
        ServerSession &sess;

        explicit Send(ServerSession &self) : sess(self) {}

        template<bool isRequest, class Body, class Fields>
        void operator()(http::message<isRequest, Body, Fields> &&msg) const {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<
                    http::message<isRequest, Body, Fields>>(std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            sess.res = sp;

            // Write the response
            http::async_write(
                    sess.derived().getStream(),
                    *sp,
                    boost::asio::bind_executor(
                            sess.strand,
                            std::bind(
                                    &ServerSession::on_write,
                                    sess.derived().shared_from_this(),
                                    std::placeholders::_1,
                                    std::placeholders::_2,
                                    sp->need_eof())));
        }
    };

    std::shared_ptr<const Resource> resource;
    http::request<http::string_body> req;
    std::shared_ptr<void> res;
    Send send;

protected:
    boost::asio::strand<boost::asio::io_context::executor_type> strand;
    boost::beast::flat_buffer buffer;

public:
    // Take ownership of the buffer
    explicit ServerSession(
            boost::asio::io_context &ioc,
            boost::beast::flat_buffer buffer,
            std::shared_ptr<const Resource> resource)
            : resource(std::move(resource)),
              send(*this),
              strand(ioc.get_executor()),
              buffer(std::move(buffer)) {}

    void do_read() {
        // Read a request
        http::async_read(
                derived().getStream(),
                buffer,
                req,
                boost::asio::bind_executor(
                        strand,
                        std::bind(
                                &ServerSession::on_read,
                                derived().shared_from_this(),
                                std::placeholders::_1,
                                std::placeholders::_2)));
    }

    void on_read(
            boost::system::error_code ec,
            std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if (ec == http::error::end_of_stream)
            return derived().do_eof();

        if (ec)
            return fail(ec, "read");

        // Send the response
        handle_request(*resource, std::move(req), send);
    }

    void on_write(
            boost::system::error_code ec,
            std::size_t bytes_transferred,
            bool close) {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail(ec, "write");

        if (close) {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return derived().do_eof();
        }

        // We're done with the response so delete it
        res = nullptr;

        // Read another request
        do_read();
    }
};

// Handles a plain HTTP connection
class PlainSession
        : public ServerSession<PlainSession>,
          public std::enable_shared_from_this<PlainSession> {
    tcp::socket socket;
    boost::asio::strand<boost::asio::io_context::executor_type> strand;

public:
    // Create the session
    PlainSession(
            tcp::socket socket,
            std::shared_ptr<const Resource> const &resource)
            : ServerSession<PlainSession>(
            socket.get_executor().context(),
            std::move(buffer),
            resource), socket(std::move(socket)), strand(socket.get_executor()) {}

    // Called by the base class
    tcp::socket &getStream() {
        return socket;
    }

    // Start the asynchronous operation
    void run() {
        do_read();
    }

    void do_eof() {
        // Send a TCP shutdown
        boost::system::error_code ec;
        socket.shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

// Accepts incoming connections and launches the sessions
class Listener : public std::enable_shared_from_this<Listener> {
    boost::asio::strand<boost::asio::io_context::executor_type> strand;
    tcp::acceptor acceptor;
    tcp::socket socket;
    std::shared_ptr<const Resource> resource;

public:
    Listener(
            boost::asio::io_context &ioc,
            tcp::endpoint endpoint,
            std::shared_ptr<const Resource> resource)
            : strand(ioc.get_executor()),
              acceptor(ioc),
              socket(ioc),
              resource(std::move(resource)) {
        boost::system::error_code ec;

        // Open the acceptor
        acceptor.open(endpoint.protocol(), ec);
        if (ec) {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if (ec) {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor.bind(endpoint, ec);
        if (ec) {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec) {
            fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void run() {
        if (!acceptor.is_open())
            return;
        do_accept();
    }

    void do_accept() {
        acceptor.async_accept(
                socket,
                std::bind(
                        &Listener::on_accept,
                        shared_from_this(), // would have been std::move(*this) if the API was single-threaded
                        std::placeholders::_1));
    }

    void on_accept(boost::system::error_code ec) {
        if (ec) {
            fail(ec, "accept");
        } else {
            // Create the ssl detector and run it
            std::make_shared<PlainSession>(
                    std::move(socket),
                    resource)->run();
        }

        // Accept another connection
        do_accept();
    }
};