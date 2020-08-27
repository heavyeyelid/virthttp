#pragma once
#include <string>
#include "json_utils.hpp"
#include "urlparser.hpp"
#include "virt_wrap.hpp"

/**
 * \internal
 * Context required to run request handlers regardless of the libvirt object type
 **/
struct HandlerContext {
    virt::Connection& conn;     ///< the connection to perform libvirt operations through (plans to change to a vector)
    JsonRes& json_res;          ///< the result of running the handlers to be sent to the client
    const TargetParser& target; ///< the incoming request's URI target

  protected:
    /**
     * \internal
     * Perfect-forwarding of #json_res's JsonRes::error
     **/
    template <class... Args>
    auto error(Args&&... args) const noexcept(noexcept(json_res.error(std::forward<Args>(args)...)))
        -> decltype(json_res.error(std::forward<Args>(args)...)) {
        return json_res.error(std::forward<Args>(args)...);
    };

    const auto fwd_err(bool fwd, int code) {
        if (!fwd)
            error(code);
        return fwd;
    };
    const auto fwd_as_if_err(int code) {
        return [&, code](const auto& arg) { return fwd_err(static_cast<bool>(arg), code); };
    };
};
