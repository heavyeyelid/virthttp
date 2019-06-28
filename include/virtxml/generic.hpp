#pragma once

#include <rapidxml_ns.hpp>

namespace virtxml {
using namespace rapidxml_ns;

template <typename T>
using Optional = T;

struct Node {
    constexpr Node(xml_node<>* node) : node(node) {}

  protected:
    xml_node<>* node;
};
}