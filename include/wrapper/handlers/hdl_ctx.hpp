#pragma once
#include <string>
#include "json_utils.hpp"
#include "virt_wrap.hpp"

struct HandlerContext {
    virt::Connection& conn;
    JsonRes& json_res;
    std::string_view key_str;
};
