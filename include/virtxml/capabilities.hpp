#pragma once

#include <rapidxml_ns.hpp>
#include <string_view>
#include <gsl/gsl>
#include "generic.hpp"
#include "xmlspan.hpp"
#include "xmlstr.hpp"

namespace virtxml {
using namespace rapidxml_ns;

struct DriverCapabilities {
  public:
    struct Host : public Node {
        struct Cpu : public Node {
            struct Feature;
            struct FeatureList : public NamedSpan<Feature> {
                constexpr explicit FeatureList(xml_node<>* node) : NamedSpan<Feature>("feature", node) {}
            };

            struct Feature : public Node {
#if 1 /* With functions */
                String name() const noexcept { return String{node->first_attribute("name")}; }
#else /* With paramexpr */
                using name(this s) = String{s.node->first_attribute("name")};
#endif
            };

            struct Features; // NOT IMPLEMENTED YET // requires autogen

            struct MigrationFeatures : public Node {
                struct UriTransport : public String {};
                struct UriTransports : public NamedSpan<UriTransport> {
                    constexpr explicit UriTransports(xml_node<>* node) : NamedSpan<UriTransport>("uri_transport", node) {}
                };

#if 1 /* With functions */
                bool live() const noexcept { return node->first_node("live") != nullptr; }
                Optional<UriTransports> uri_transports() const noexcept { return UriTransports{node->first_node("uri_transports")}; }
#else /* With paramexpr */
                using live(this s) = s.node->first_node("live") != nullptr;
                using uri_transports(this s) = UriTransports{s.node->first_node("uri_transports")};
#endif
            };

#if 1 /* With functions */
            String arch() const noexcept { return String{node->first_node("arch")}; }
            // Optional<String> model() const noexcept; // autogen from https://github.com/wiedi/libvirt/blob/master/src/cpu/cpu_map.xml
            FeatureList feature_list() const noexcept { return FeatureList{node}; }
#else /* With paramexpr */
            using arch(this s) = String{s.node->first_node("arch")};
            using model(this s) = /* autogen + TMP magic */;
            using feature_list(this s) = FeatureList{s.node};
#endif
        };

        /*
        void uuid(std::string_view sv) {
            auto pool_val = node->document()->allocate_string(sv.data(), sv.size());
            if(auto uuid_node = node->first_node("uuid"); uuid_node != nullptr){
                uuid_node->name(pool_val, sv.size());
                return;
            }
            node->insert_node(nullptr, node->document()->allocate_node(node_element, "uuid", pool_val, 0, sv.size()));
        }
         */

#if 1 /* With functions */
        String uuid() const noexcept { return String{node->first_node("uuid")}; }
        // String uuid() noexcept { return {node->first_node("uuid")}; }
#else /* With paramexpr */
        using uuid(this s) = String{s.node->first_node("uuid")};
        using cpu() = Cpu{s.node->first_node("cpu")};
#endif
    };
    struct Guest;
    struct GuestList : public NamedSpan<Guest> {
        constexpr explicit GuestList(xml_node<>* node) : NamedSpan<Guest>("guest", node) {}
    };

    struct Guest : public Node {
#if 1 /* With functions */
#else /* With paramexpr */
#endif
    };

    explicit DriverCapabilities(gsl::zstring<> xml) { doc.parse<0>(xml); }

#if 1 /* With functions */
  private:
    // inline auto cap() { return doc.first_node("capabilities"); }
    inline auto cap() const { return doc.first_node("capabilities"); }

  public:
    // inline auto host() { return Host{cap()->first_node("host")}; }
    inline Host host() const { return {cap()->first_node("host")}; }
    inline GuestList guest_list() { return GuestList{cap()}; }
#else /* With paramexpr */
  private
    using cap(this s) = s.doc.first_node("capabilities");
  public
    using host(this s) = Host{s.cap()->first_node("host")};
    using guest_list(this s) = GuestList{s.cap()->};
#endif
  private:
    xml_document<> doc{};
};
} // namespace virtxml

#if 1 /* With functions */
#else /* With paramexpr */
#endif