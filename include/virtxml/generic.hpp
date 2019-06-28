#pragma once

#include <rapidxml_ns.hpp>
#include "xmlval.hpp"

namespace virtxml {
using namespace rapidxml_ns;

template <typename T> using Optional = T;

struct Node {
    constexpr Node(xml_node<>* node) : node(node) {}
#if 1 /* With functions */
    constexpr explicit operator bool() const noexcept { return node != nullptr; }
#else /* With paramexpr */
    using operator bool(this s) = s.item != nullptr;
#endif
  protected:
    xml_node<>* node;
};
}