#pragma once

#include <rapidxml_ns.hpp>
#include <string_view>

namespace virtxml {
using namespace rapidxml_ns;

struct String {
    template <typename T> constexpr explicit String(T* item) : item(item) {}

#if 1 /* With functions */

    constexpr explicit operator bool() const noexcept { return item != nullptr; }
    inline explicit operator std::string_view() const noexcept { return {item->value(), item->value_size()}; }

#else /* With paramexpr */

    using operator bool(this s) = s.item != nullptr;
    using operator std::string_view(this s) = std::string_view{item->value(), item->value_size()};

#endif

  private:
    xml_base<>* item;
};
} // namespace virtxml