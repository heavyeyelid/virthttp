#pragma once

#include <rapidxml_ns.hpp>
#include <string_view>
#include <gsl/gsl>
#include "generic.hpp"
#include "xmlspan.hpp"
#include "xmlval.hpp"

namespace virtxml {
using namespace rapidxml_ns;
inline namespace {

struct DriverCapabilities {
  public:
    struct Host : public Node {
        struct Cpu : public Node {
            struct Model : public Node {
#if 1 /* With functions */
                constexpr explicit operator String() const noexcept { return String{node}; }
                Optional<String> fallback_str() const noexcept { return String{node->first_attribute("fallback")}; }
#else /* With paramexpr */
                using operator String(this s) = String{s.node};
                using fallback_str() = String{node->first_attribute("fallback")};
#endif
            };

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

            struct Features; // NOT IMPLEMENTED YET // requires autogen from https://github.com/wiedi/libvirt/blob/master/src/cpu/cpu_map.xml

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

            struct Topology : public Node {
#if 1 /* With functions */
                Optional<Integral> sockets() const noexcept { return Integral{node->first_attribute("sockets")}; }
                Optional<Integral> cores() const noexcept { return Integral{node->first_attribute("cores")}; }
                Optional<Integral> threads() const noexcept { return Integral{node->first_attribute("threads")}; }
#else /* With paramexpr */
                using sockets(this s) = Integral{node->first_attribute("sockets")};
                using cores(this s) = Integral{node->first_attribute("cores")};
                using threads(this s) = Integral{node->first_attribute("threads")};
#endif
            };

            struct SecModel : public Node {
#if 1 /* With functions */
                String model() const noexcept { return String{node->first_node("model")}; }
                Integral doi() const noexcept { return Integral{node->first_node("doi")}; }
#else /* With paramexpr */
                using model(this s) = String{s.node->first_node("model")};
                using doi(this s) = Integral{s.node->first_node("doi")};
#endif
            };

#if 1 /* With functions */
            String arch() const noexcept { return String{node->first_node("arch")}; }
            Optional<String> vendor() const noexcept { return String{node->first_node("vendor")}; }
            Optional<String> model() const noexcept {}
            FeatureList feature_list() const noexcept { return FeatureList{node}; }
            Optional<MigrationFeatures> migration_features() const noexcept { return MigrationFeatures{node->first_node("migration_features")}; }
            Optional<SecModel> secmodel() const noexcept { return SecModel{node->first_node("secmodel")}; }
#else /* With paramexpr */
            using arch(this s) = String{s.node->first_node("arch")};
            using arch(this s) = String{s.node->first_node("vendor")};
            using model(this s) = Node{s.node->first_node("model")};
            using feature_list(this s) = FeatureList{s.node};
            using migration_features(this s) = MigrationFeatures{s.node->first_node("migration_features")};
            using secmodel(this s) = SecModel{s.node->first_node("migration_features")};
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
} // namespace
} // namespace virtxml

#if 1 /* With functions */
#else /* With paramexpr */
#endif