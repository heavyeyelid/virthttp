#pragma once

#include <string_view>
#include <rapidxml_ns.hpp>

namespace virtxml {
using namespace rapidxml_ns;

struct Value {
    constexpr Value(xml_base<>* item) : item(item) {}
#if 1 /* With functions */
    constexpr explicit operator bool() const noexcept { return item != nullptr; }
#else /* With paramexpr */
    using operator bool(this s) = s.item != nullptr;
#endif
  protected:
    xml_base<>* item;
};

struct String : public Value {
#if 1 /* With functions */
    inline explicit operator std::string_view() const noexcept { return {item->value(), item->value_size()}; }
#else /* With paramexpr */
    using operator std::string_view(this s) = std::string_view{item->value(), item->value_size()};
#endif
};

struct Integral : public Value {
#if 1 /* With functions */
    inline explicit operator int() const noexcept { return std::atoi(item->value()); }
#else /* With paramexpr */
    using operator int(this s) = std::atoi(s.item->value());
#endif
};
} // namespace virtxml