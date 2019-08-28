#pragma once
#include <rapidjson/document.h>
#include "virt_wrap/TypesParam.hpp"

std::optional<virt::TypedParams> json_to_typed_parameters(const rapidjson::Value& val);