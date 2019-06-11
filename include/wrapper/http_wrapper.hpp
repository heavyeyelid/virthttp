//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#pragma once

#define RAPIDJSON_HAS_STDSTRING 1

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <urlparser.hpp>
#include <vector>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/config.hpp>
#include <libvirt/libvirt.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "virt_wrap.hpp"
#include "virt_wrap/Connection.hpp"
#include "virt_wrap/Domain.hpp"
#include "virt_wrap/TypesParam.hpp"
#include "virt_wrap/impl/Connection.hpp"
#include "virt_wrap/impl/Domain.hpp"
#include "virt_wrap/impl/TypedParams.hpp"
#include "handler.hpp"
#include "fwd.hpp"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http  = beast::http;          // from <boost/beast/http.hpp>
namespace net   = boost::asio;          // from <boost/asio.hpp>
using tcp       = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

// Return a reasonable mime type based on the extension of a file.
beast::string_view mime_type(beast::string_view path) {
    using beast::iequals;
    auto const ext = [&path] {
        auto const pos = path.rfind(".");
        if (pos == beast::string_view::npos)
            return beast::string_view{};
        return path.substr(pos);
    }();
    if (iequals(ext, ".htm"))
        return "text/html";
    if (iequals(ext, ".html"))
        return "text/html";
    if (iequals(ext, ".php"))
        return "text/html";
    if (iequals(ext, ".css"))
        return "text/css";
    if (iequals(ext, ".txt"))
        return "text/plain";
    if (iequals(ext, ".js"))
        return "application/javascript";
    if (iequals(ext, ".json"))
        return "application/json";
    if (iequals(ext, ".xml"))
        return "application/xml";
    if (iequals(ext, ".swf"))
        return "application/x-shockwave-flash";
    if (iequals(ext, ".flv"))
        return "video/x-flv";
    if (iequals(ext, ".png"))
        return "image/png";
    if (iequals(ext, ".jpe"))
        return "image/jpeg";
    if (iequals(ext, ".jpeg"))
        return "image/jpeg";
    if (iequals(ext, ".jpg"))
        return "image/jpeg";
    if (iequals(ext, ".gif"))
        return "image/gif";
    if (iequals(ext, ".bmp"))
        return "image/bmp";
    if (iequals(ext, ".ico"))
        return "image/vnd.microsoft.icon";
    if (iequals(ext, ".tiff"))
        return "image/tiff";
    if (iequals(ext, ".tif"))
        return "image/tiff";
    if (iequals(ext, ".svg"))
        return "image/svg+xml";
    if (iequals(ext, ".svgz"))
        return "image/svg+xml";
    return "application/text";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(beast::string_view base, beast::string_view path) {
    if (base.empty())
        return path.to_string();
    std::string result = base.to_string();
#if BOOST_MSVC
    char constexpr path_separator = '\\';
    if(result.back() == path_separator)
      result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for(auto& c : result)
      if(c == '/')
        c = path_separator;
#else
    char constexpr path_separator = '/';
    if (result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template <class Body, class Allocator, class Send>
void handle_request(beast::string_view doc_root, http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
  // Returns a bad request response
  const auto bad_request = [&](beast::string_view why) {
    http::response<http::string_body> res{http::status::bad_request, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = why.to_string();
    res.prepare_payload();
    return res;
  };

  logger.info("Received from a Session: HTTP ", boost::beast::http::to_string(req.method()), ' ', req.target());

  // Returns a not found response
  const auto not_found = [&](beast::string_view target) {
    http::response<http::string_body> res{http::status::not_found, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "The resource '" + target.to_string() + "' was not found.";
    res.prepare_payload();
    return res;
  };

  // Returns a server error response
  auto const server_error = [&req](beast::string_view what) {
    http::response<http::string_body> res{http::status::internal_server_error, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred: '" + what.to_string() + "'";
    res.prepare_payload();
    return res;
  };

  // Make sure we can handle the method
  if(req.method() != http::verb::get && req.method() != http::verb::patch && req.method() != http::verb::post
     && req.method() != http::verb::put && req.method() != http::verb::head)
    return send(bad_request("Unknown HTTP-method"));

  // Request path must be absolute and not contain "..".
  if(req.target().empty() || req.target()[0] != '/' || req.target().find("..") != beast::string_view::npos)
    return send(bad_request("Illegal request-target"));

  std::size_t size = 0;
  std::string json_string{};

    auto buffer = getResult(std::move(req));

    size = buffer.GetSize();
    json_string = buffer.GetString();

  // Build the path to the requested file
  /*
  std::string path = path_cat(doc_root, req.target());
  if(req.target().back() == '/')
      path.append("index.html");

  // Attempt to open the file
  beast::error_code ec;
  http::file_body::value_type body;
  body.open(path.c_str(), beast::file_mode::scan, ec);


  // Handle the case where the file doesn't exist
  if(ec == beast::errc::no_such_file_or_directory)
      return send(not_found(req.target()));

  // Handle an unknown error
  if(ec)
      return send(server_error(ec.message()));

  // Cache the size since we need it after the move
  auto const size = body.size();
  */



  // Respond to HEAD request
  if(req.method() == http::verb::head) {
    http::response<http::empty_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return send(std::move(res));
  }

  // Respond to GET request
  http::response<http::string_body> res{http::status::ok, req.version()};
  res.body() = json_string;
  res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  res.set(http::field::content_type, "application/json");
  res.content_length(size);
  res.keep_alive(req.keep_alive());
  return send(std::move(res));
}

//------------------------------------------------------------------------------

// Report a failure
void fail(beast::error_code ec, const char* what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

// Handles an HTTP server connection
class Session : public std::enable_shared_from_this<Session> {
    // This is the C++11 equivalent of a generic lambda.
    // The function object is used to send an HTTP message.
    struct send_lambda {
        Session &self_;

    explicit send_lambda(Session& self) : self_(self) {}

    template <bool isRequest, class Body, class Fields>
    void operator()(http::message<isRequest, Body, Fields>&& msg) const {
      // The lifetime of the message has to extend
      // for the duration of the async operation so
      // we use a shared_ptr to manage it.
      auto sp = std::make_shared<http::message<isRequest, Body, Fields>>(std::move(msg));

      // Store a type-erased version of the shared
      // pointer in the class to keep it alive.
      self_.res_ = sp;

      // Write the response
      http::async_write(
       self_.socket_,
       *sp,
       net::bind_executor(
        self_.strand_,
        std::bind(
         &Session::on_write, self_.shared_from_this(), std::placeholders::_1, std::placeholders::_2, sp->need_eof())));
    }
  };

  tcp::socket socket_;
  net::strand<net::io_context::executor_type> strand_;
  beast::flat_buffer buffer_;
  std::shared_ptr<std::string const> doc_root_;
  http::request<http::string_body> req_;
  std::shared_ptr<void> res_;
  send_lambda lambda_;

public:
  // Take ownership of the socket
  explicit Session(tcp::socket socket, std::shared_ptr<std::string const> const& doc_root) :
    socket_(std::move(socket)),
    strand_(socket_.get_executor()),
    doc_root_(doc_root),
    lambda_(*this) {}

  // Start the asynchronous operation
  void run() {
    do_read();
  }

  void do_read() {
    // Make the request empty before reading,
    // otherwise the operation behavior is undefined.
    req_ = {};

    // Read a request
    http::async_read(
     socket_,
     buffer_,
     req_,
     net::bind_executor(strand_,
                        std::bind(&Session::on_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2)));
  }

  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    // This means they closed the connection
    if(ec == http::error::end_of_stream)
      return do_close();

    if(ec)
      return fail(ec, "read");

    // Send the response
    handle_request(*doc_root_, std::move(req_), lambda_);
  }

  void on_write(beast::error_code ec, std::size_t bytes_transferred, bool close) {
    boost::ignore_unused(bytes_transferred);

    if(ec)
      return fail(ec, "write");

    if(close) {
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
    beast::error_code ec;
    socket_.shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
  }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class Listener : public std::enable_shared_from_this<Listener> {
  tcp::acceptor acceptor_;
  tcp::socket socket_;
  std::shared_ptr<const std::string> doc_root_;

public:
  Listener(net::io_context& ioc, tcp::endpoint endpoint, const std::shared_ptr<const std::string>& doc_root) :
    acceptor_(ioc),
    socket_(ioc),
    doc_root_(doc_root) {
    beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if(ec) {
      fail(ec, "open");
      return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if(ec) {
      fail(ec, "set_option");
      return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if(ec) {
      fail(ec, "bind");
      return;
    }

    // Start listening for connections
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if(ec) {
      fail(ec, "listen");
      return;
    }
  }

  // Start accepting incoming connections
  void run() {
    if(!acceptor_.is_open())
      return;
    do_accept();
  }

  void do_accept() {
    acceptor_.async_accept(socket_, std::bind(&Listener::on_accept, shared_from_this(), std::placeholders::_1));
  }

  void on_accept(beast::error_code ec) {
    if(ec)
      fail(ec, "accept");
    else {
      // Create the Session and run it
      std::make_shared<Session>(std::move(socket_), doc_root_)->run();
    }

    // Accept another connection
    do_accept();
  }
};