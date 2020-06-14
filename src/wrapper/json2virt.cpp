#include "wrapper/json2virt.hpp"

std::optional<virt::TypedParams> json_to_typed_parameters(const boost::json::value& obj) {
    virt::TypedParams ret{};
    if (!obj.is_object())
        return std::nullopt;
    for (const auto& key_val : obj.get_object()) {
        const auto key = key_val.key_c_str();
        const auto& val = key_val.value();
        if (val.is_structured())
            return std::nullopt;
        switch (val.kind()) {
            using boost::json::kind;
        case kind::null:
            [[fallthrough]];
        case kind::object:
            [[fallthrough]];
        case kind::array:
            return std::nullopt;
        case kind::bool_:
            ret.add(key, val.get_bool());
            break;
        case kind::string: {
            const auto str = val.get_string();
            ret.add(key, str.c_str());
        } break;
        case kind::double_:
            ret.add(key, val.get_double());
            break;
        case kind::int64:
            ret.add(key, static_cast<long long int>(val.get_int64()));
            break;
        case kind::uint64:
            ret.add(key, static_cast<unsigned long long int>(val.get_uint64()));
            break;
        }
    }
    return {std::move(ret)};
}
