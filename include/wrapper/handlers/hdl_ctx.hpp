#pragma once
#include <string>
#include "json_utils.hpp"
#include "urlparser.hpp"
#include "virt_wrap.hpp"

struct HandlerContext {
    virt::Connection& conn;
    JsonRes& json_res;
    TargetParser& target;
    std::string_view key_str;

  protected:
    template <class... Args> auto error(Args... args) const noexcept { return json_res.error(args...); };
};
