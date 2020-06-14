#pragma once
#include <boost/json.hpp>
#include <virt_wrap/Error.hpp>
#include <virt_wrap/utility.hpp>
#include "wrapper/error_msg.hpp"

/**
 * \internal
 * JSON Response
 **/
struct JsonRes : public boost::json::object {
  private:
    [[nodiscard]] decltype(auto) json() const noexcept { return static_cast<const boost::json::object&>(*this); };
    [[nodiscard]] decltype(auto) json() noexcept { return static_cast<boost::json::object&>(*this); };

  public:
    JsonRes() {
        using boost::json::array_kind;
        emplace("results", array_kind);
        emplace("success", true);
        emplace("errors", array_kind);
        emplace("messages", array_kind);
    }

    /**
     * \internal
     * Append a result
     **/
    template <typename T> void result(T&& val) { (*this)["results"].get_array().push_back(val); }
    /**
     * \internal
     * Append a message
     **/
    template <typename T> void message(T&& val) { (*this)["messages"].get_array().push_back(val); }
    /**
     * \internal
     * Append an error
     *
     * \param[in] code virthttp error code; see ErrorMessages
     **/
    void error(int code) {
        (*this)["success"] = false;
        boost::json::object err{};
        err.emplace("code", code);
        err.emplace("message", error_messages[code]);

        if (auto vir_err = virt::extractLastError(); vir_err)
            err.emplace("libvirt", vir_err.message);

        (*this)["errors"].get_array().push_back(err);
    };
};