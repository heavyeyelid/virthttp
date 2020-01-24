#pragma once
#include <string>
#include "hdl_ctx.hpp"
#include "json_utils.hpp"
#include "virt_wrap.hpp"

/**
 * \internal
 * \file
 * Base elements for libvirt objects-specific handlers
 **/

/**
 * \internal
 * Template variadic HTTP verb pack wrapper
 *
 * \tparam Vs the pack of HTTP verbs
 **/
template <boost::beast::http::verb... Vs> struct VerbList {

    /**
     * \internal
     * constexpr-usable array of HTTP verbs from the class' template arguments
     **/
    constexpr static std::array<boost::beast::http::verb, sizeof...(Vs)> values = {Vs...}; // note: cannot use CTAD because of the empty list case
};

/**
 * \internal
 * Purely abstract base class for libvirt object-specific handlers
 *
 * Intended to be used in a place where the compiler should definitely be able to de-virtualize
 **/
struct HandlerMethods : public HandlerContext {

    /**
     * \internal
     * Creates an associated libvirt object.
     * Is the handler for HTTP POST
     *
     * \param[in] obj the HTTP request body, parsed as JSON
     * \return DependsOutcome::SUCCESS or DependsOutcome::FAILURE
     * */
    virtual DependsOutcome create(const rapidjson::Value& obj) = 0;
    /**
     * \internal
     * Retrieves information about the associated libvirt object(s)
     * Is the handler for HTTP GET
     *
     * \param[in] obj the HTTP request body, parsed as JSON
     * \return DependsOutcome::SUCCESS or DependsOutcome::FAILURE
     * */
    virtual DependsOutcome query(const rapidjson::Value& obj) = 0;
    /**
     * \internal
     * Performs the described modification on the associated libvirt object(s)
     * Is the handler for HTTP PATCH
     *
     * \param[in] obj the HTTP request body, parsed as JSON
     * \return DependsOutcome::SUCCESS or DependsOutcome::FAILURE
     * */
    virtual DependsOutcome alter(const rapidjson::Value& obj) = 0;
    /**
     * \internal
     * Removes the associated libvirt object(s)
     * Is the handler for HTTP DELETE
     *
     * \param[in] obj the HTTP request body, parsed as JSON
     * \return DependsOutcome::SUCCESS or DependsOutcome::FAILURE
     * */
    virtual DependsOutcome vacuum(const rapidjson::Value& obj) = 0;

    /**
     * \internal
     * Array of HTTP verbs handled by the handler methods
     **/
    using Verbs =
        VerbList<boost::beast::http::verb::post, boost::beast::http::verb::get, boost::beast::http::verb::patch, boost::beast::http::verb::delete_>;
    /**
     * \internal
     * Array of the handler methods, with corresponding indices to the handled verb in #Verbs
     **/
    constexpr static std::array methods = {&HandlerMethods::create, &HandlerMethods::query, &HandlerMethods::alter, &HandlerMethods::vacuum};

    static_assert(Verbs::values.size() == methods.size());

    /**
     * \internal
     * \param hc the `HandlerContext` to use as a base
     **/
    explicit constexpr HandlerMethods(HandlerContext& hc) noexcept : HandlerContext(hc) {}

    /**
     * \internal
     * Gives index of a given HTTP verb in #Verbs, which can be used to access the associated handler in #methods
     *
     * \param v the HTTP verb
     * \return the index of the verb if found, -1 otherwise
     **/
    constexpr static long verb_to_idx(boost::beast::http::verb v) noexcept {
        const auto it = cexpr::find(Verbs::values.begin(), Verbs::values.end(), v);
        return it != Verbs::values.end() ? std::distance(Verbs::values.begin(), it) : -1;
    }
};