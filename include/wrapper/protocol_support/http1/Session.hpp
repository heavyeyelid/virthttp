#pragma once

#include <memory>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "../../general_store.hpp"
#include "../beast_internals.hpp"
#include "../request_handler.hpp"

// Handles an HTTP1 server connection
class Session : public std::enable_shared_from_this<Session> {
    // This is the C++11 equivalent of a generic lambda.
    // The function object is used to send an HTTP message.
    struct SendLambda {
        Session& self_;

        explicit SendLambda(Session& self) : self_(self) {}

        template <bool isRequest, class Body, class Fields> void operator()(boost::beast::http::message<isRequest, Body, Fields>&& msg) const {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<boost::beast::http::message<isRequest, Body, Fields>>(std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            self_.res_ = sp;

            // Write the response
            boost::beast::http::async_write(
                self_.socket_, *sp,
                boost::beast::net::bind_executor(self_.strand_, std::bind(&Session::on_write, self_.shared_from_this(), std::placeholders::_1,
                                                                          std::placeholders::_2, sp->need_eof())));
        }
    };

    boost::beast::net::ip::tcp::socket socket_;
    boost::beast::net::strand<boost::beast::net::ip::tcp::socket::executor_type> strand_;
    boost::beast::flat_buffer buffer_;
    std::reference_wrapper<GeneralStore> m_gstore;
    boost::beast::http::request<boost::beast::http::string_body> req_;
    std::shared_ptr<void> res_;
    SendLambda lambda_;

  public:
    // Take ownership of the socket
    explicit Session(boost::beast::net::ip::tcp::socket socket, GeneralStore& gstore)
        : socket_(std::move(socket)), strand_(socket_.get_executor()), m_gstore(gstore), lambda_(*this) {}

    // Start the asynchronous operation
    void run() { do_read(); }

    void do_read() {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req_ = {};

        // Read a request
        boost::beast::http::async_read(socket_, buffer_, req_,
                                       boost::beast::net::bind_executor(
                                           strand_, std::bind(&Session::on_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2)));
    }

    void on_read(boost::beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if (ec == boost::beast::http::error::end_of_stream)
            return do_close();

        if (ec)
            return fail(ec, "read");

        // Send the response
        handle_request(m_gstore, std::move(req_), lambda_);
    }

    void on_write(boost::beast::error_code ec, std::size_t bytes_transferred, bool close) {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail(ec, "write");

        if (close) {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return do_close();
        }

        // We're done with the response so delete it
        res_ = nullptr;

        // Read another request
        do_read();
    }

    void do_close() {
        // Send a TCP shutdown
        boost::beast::error_code ec;
        socket_.shutdown(boost::beast::net::ip::tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};
