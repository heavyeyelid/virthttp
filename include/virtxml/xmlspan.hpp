#pragma once

#include <gsl/gsl>
#include <rapidxml_ns.hpp>
#include "generic.hpp"

namespace virtxml {
using namespace rapidxml_ns;
template <typename C, typename T> class NamedSpan;

namespace impl {
template <typename C, typename T> class NamedSpanIt {
    friend NamedSpan<C, T>;
    xml_node<T>* node;

  public:
    auto& operator++() noexcept {
        node = node->next_sibling(node->name(), node->name_size());
        return *this;
    }

    const auto operator++(int) noexcept {
        const auto old = NamedSpanIt{node};
        node = node->next_sibling(node->name(), node->name_size());
        return old;
    }
    auto& operator--() noexcept {
        node = node->previous_sibling(node->name(), node->name_size());
        return *this;
    }

    const auto operator--(int) noexcept {
        const auto old = NamedSpanIt{node};
        node = node->previous_sibling(node->name(), node->name_size());
        return old;
    }

#if 1 /* With functions */

    C operator*() { return C{node}; }
    C operator*() const { return C{node}; }

#else /* With paramexpr */

    using operator*() = C{node};

#endif

    constexpr static NamedSpanIt end = {nullptr};
};

} // namespace impl

template <typename C, typename T = char> class NamedSpan : public Node {
  protected:
    gsl::czstring<> name;

  public:
    constexpr explicit NamedSpan(gsl::czstring<> name, xml_node<T>* node) : name(name), Node(node){};

#if 1 /* With functions */

    auto begin() const { return impl::NamedSpanIt<C, T>{node->first_node(name)}; }
    auto begin() { return impl::NamedSpanIt<C, T>{node->first_node(name)}; }
    auto rbegin() const { return std::make_reverse_iterator(impl::NamedSpanIt<C, T>{node->last_node(name)}); }
    auto rbegin() { return std::make_reverse_iterator(impl::NamedSpanIt<C, T>{node->last_node(name)}); }
    constexpr auto end() const noexcept { return impl::NamedSpanIt<C, T>::end; }
    constexpr auto rend() const noexcept { return impl::NamedSpanIt<C, T>::end; }

#else /* With paramexpr */

    using begin(this s) = node != nullptr ? impl::NamedSpanIt<C, T>{s.node->first_node(name)} : end();
    using end() = impl::NamedSpanIt<C, T>::end;

#endif
};
} // namespace virtxml
