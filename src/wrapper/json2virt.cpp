#include "wrapper/json2virt.hpp"

std::optional<virt::TypedParams> json_to_typed_parameters(const rapidjson::Value& obj) {
    virt::TypedParams ret{};
    if (!obj.IsObject())
        return std::nullopt;
    for (auto it = obj.MemberBegin(); it != obj.MemberEnd(); ++it) {
        const auto& val = *it;
        if (val.value.IsObject() || val.value.IsArray())
            return std::nullopt;
        switch (val.value.GetType()) {
        case rapidjson::kNullType:
            return std::nullopt;
        case rapidjson::kFalseType:
            ret.add(val.name.GetString(), false);
            break;
        case rapidjson::kTrueType:
            ret.add(val.name.GetString(), true);
            break;
        case rapidjson::kObjectType:
        case rapidjson::kArrayType:
            return std::nullopt;
        case rapidjson::kStringType:
            ret.add(val.name.GetString(), val.value.GetString());
            break;
        case rapidjson::kNumberType:
            if (val.value.IsDouble() || val.value.IsLosslessDouble())
                ret.add(val.name.GetString(), val.value.GetDouble());
            else if (val.value.IsFloat() || val.value.IsLosslessFloat())
                ret.add(val.name.GetString(), val.value.GetFloat());
            else if (val.value.IsInt64())
                ret.add(val.name.GetString(), static_cast<long long int>(val.value.GetInt64()));
            else if (val.value.GetUint64())
                ret.add(val.name.GetString(), static_cast<unsigned long long int>(val.value.GetUint64()));
            else if (val.value.IsUint())
                ret.add(val.name.GetString(), val.value.GetUint());
            else if (val.value.IsInt())
                ret.add(val.name.GetString(), val.value.GetInt());
            break;
        }
    }
    return {std::move(ret)};
}
