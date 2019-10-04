#pragma once
#include <boost/beast.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "../general_store.hpp"
#include "../handler.hpp"
#include "../handlers/async/async_handler.hpp"
#include "urlparser.hpp"

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template <class Body, class Allocator, class Send>
void handle_request(GeneralStore& gstore, boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& req, Send&& send) {
    // Returns a bad request response
    const auto bad_request = [&](boost::beast::string_view why) {
        boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::bad_request, req.version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = why.to_string();
        res.prepare_payload();
        return res;
    };

    logger.info("Received from a Session: HTTP ", boost::beast::http::to_string(req.method()), ' ', req.target());

    // Returns a not found response
    const auto not_found = [&](boost::beast::string_view target) {
        boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::not_found, req.version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "The resource '" + target.to_string() + "' was not found.";
        res.prepare_payload();
        return res;
    };

    // Returns a server error response
    auto const server_error = [&req](boost::beast::string_view what) {
        boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::internal_server_error, req.version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + what.to_string() + "'";
        res.prepare_payload();
        return res;
    };

    constexpr std::array supported_methods = {boost::beast::http::verb::get, boost::beast::http::verb::patch, boost::beast::http::verb::post,
                                              boost::beast::http::verb::delete_};
    // Make sure we can handle the method
    if (std::find(supported_methods.begin(), supported_methods.end(), req.method()) == supported_methods.end())
        return send(bad_request("Unknown/Unsupported HTTP-method"));

    // Request path must be absolute and not contain "..".
    if (req.target().empty() || req.target()[0] != '/' || req.target().find("..") != boost::beast::string_view::npos)
        return send(bad_request("Illegal request-target"));

    const auto forward_packid = [&](auto& res) noexcept {
        if (const auto pakid = req["X-Packet-ID"]; !pakid.empty())
            res.set("X-Packet-ID", pakid);
    };

    auto req_method = req.method();
    auto target = TargetParser{bsv2stdsv(req.target())};
    const auto& path_parts = target.getPathParts();

    if (path_parts.empty())
        return send(bad_request("No module name specified"));

    // Handle cases where the client wants to retrieve an async result
    if (path_parts[0] == "async") {
        if (path_parts.size() != 2)
            return send(bad_request("Bad invalid request target"));

        if (auto opt = target.getBool("async"); opt && *opt)
            return send(bad_request("Async retrieve cannot be async'ed"));

        auto [code, body] = handle_async_retrieve<TransportProto::HTTP1>(gstore, path_parts[1]);

        boost::beast::http::response<boost::beast::http::string_body> res{code, req.version()};
        res.content_length(body.size());
        res.body() = std::move(body);
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "application/json");
        forward_packid(res);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }

    if (auto opt = target.getBool("async"); opt && *opt) {
        auto launch_res = gstore.async_store.launch([&gstore, target = std::move(target), req = std::move(req)]() {
            auto buf = handle_json(gstore, req, target);
            return std::string{buf.GetString(), buf.GetLength()};
        });

        if (!launch_res)
            return send(server_error("Unable to enqueue async request"));

        constexpr auto n_bytes = sizeof(AsyncStore::IndexType) * 2;

        boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::ok, req.version()};
        res.body() = std::string{hex_encode_id(*launch_res).data(), n_bytes};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "application/json");
        forward_packid(res);
        res.content_length(n_bytes);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }

    auto buffer = handle_json(gstore, req, std::move(target));

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
    if (req_method == boost::beast::http::verb::head) {
        boost::beast::http::response<boost::beast::http::empty_body> res{boost::beast::http::status::ok, req.version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "application/json");
        forward_packid(res);
        res.content_length(std::size_t{buffer.GetSize()});
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }

    boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::ok, req.version()};
    res.body() = std::string{buffer.GetString()};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "application/json");
    forward_packid(res);
    res.content_length(std::size_t{buffer.GetSize()});
    res.keep_alive(req.keep_alive());
    return send(std::move(res));
}
