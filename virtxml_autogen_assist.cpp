#include <fstream>
#include <iostream>
#include <sstream>
#include <rapidxml_ns.hpp>
#include "virtxml/xmlspan.hpp"

using namespace std::literals;
using namespace rapidxml_ns;

struct StreamCapitalize {
    const char* str;
};
std::ostream& operator<<(std::ostream& os, StreamCapitalize sc){
    if(sc.str != nullptr && *sc.str != '\0')
        os << (char)toupper(sc.str[0]) << sc.str + 1;
    return os;
}

struct StreamGenStringFcn {
    const char* str;
};
std::ostream& operator<<(std::ostream& os, StreamGenStringFcn gsf){
    return os << "String " << gsf.str << "() const noexcept { return String{node->first_node(\"" << gsf.str << "\")}; }\n";
}

struct StreamGenIntegralFcn {
    const char* str;
};
std::ostream& operator<<(std::ostream& os, StreamGenIntegralFcn gsf){
    return os << "Integral " << gsf.str << "() const noexcept { return String{node->first_node(\"" << gsf.str << "\")}; }\n";
}

struct StreamGenBoolFcn {
    const char* str;
};
std::ostream& operator<<(std::ostream& os, StreamGenBoolFcn gsf){
    return os << "Integral " << gsf.str << "() const noexcept { return node->first_node(\"" << gsf.str << "\") != nullptr; }\n";
}

enum Marks {
    None = 0,
    Optional = 1,
    ZeroOrMore = 2,
    OneOrMore = 3,
};

bool verify(const xml_document<>& doc) noexcept {
    const auto gram_node = doc.first_node();
    if (!gram_node)
        return false;
    if (gram_node->name() != "grammar"sv)
        return false;
    if (const auto attr = gram_node->first_attribute("xmlns"); !attr || attr->value() != "http://relaxng.org/ns/structure/1.0"sv)
        return false;
    if (const auto attr = gram_node->first_attribute("datatypeLibrary"); !attr || attr->value() != "http://www.w3.org/2001/XMLSchema-datatypes"sv)
        return false;

    return gram_node->first_node("start");
}

const xml_node<>& rng_deref(const xml_node<>& n, virtxml::NamedSpan<xml_node<>> definitions) {
    const auto ref_name = n.first_attribute("name")->value();
    const auto it = std::find_if(definitions.begin(), definitions.end(),
                                 [=](const xml_node<>& def) { return std::strcmp(def.first_attribute("name")->value(), ref_name) == 0; });
    return (*it).first_node();
};

void walk(const xml_node<>& n, unsigned marks, gsl::czstring<> scope, virtxml::NamedSpan<xml_node<>> definitions, std::ostream& of) {
    auto walk = [&](const xml_node<>& n, unsigned marks = Marks::None, gsl::czstring<> ss = nullptr) { return ::walk(n, marks, ss, definitions, of); };
    auto rng_deref = [&](const xml_node<>& n) -> decltype(auto) { return ::rng_deref(n, definitions); };

    if (n.name() == "ref"sv) {
        walk(rng_deref(n), Marks::None);
    } else if (n.name() == "element"sv) {
        const auto element_tag = scope ? scope : n.first_attribute("name")->value();

        std::stringstream class_ss{};
        bool class_required = true;
        for (auto it = n.first_node(); it != nullptr; it = it->next_sibling()) {
            auto& sub = *it;
            if (sub.name() == "empty"sv) {
                class_required = false;
                if (!(marks & Marks::Optional))
                    std::cerr << "Warning: Empty tag in non-optional elem" << std::endl;
                of << StreamGenBoolFcn{element_tag};
                break;
            } else if (sub.name() == "text"sv) {
                class_required = false;
                of << StreamGenStringFcn{element_tag};
            } else if (sub.name() == "interleave"sv) {
                ::walk(sub, marks, element_tag, definitions, class_ss);
            } else if (sub.name() == "optional"sv){
                ::walk(sub, marks & Marks::Optional, element_tag, definitions, class_ss);
            } else if(sub.name() == "zeroOrMore"sv) {
                ::walk(sub, marks & Marks::ZeroOrMore, element_tag, definitions, class_ss);
            }
        }
        if(class_required){
            of << "struct " << StreamCapitalize{element_tag} << " : public Node {\n";
            of << class_ss.rdbuf();
            of << "};\n";
        }
    } else if (n.name() == "attribute"sv) {

    } else if (n.name() == "data"sv) {
    }
}

void process(const xml_document<>& doc, std::ofstream of) {
    const auto gram_node = doc.first_node("grammar");
    virtxml::NamedSpan<xml_node<>> definitions{"define", gram_node};

    const auto start = gram_node->first_node("start");
    walk(*start, Marks::None, nullptr, definitions, of);
}

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " [infile] [outfile]" << std::endl;
        return 0;
    }

    rapidxml_ns::xml_document<> doc{};
    if (!verify(doc)) {
        std::cerr << "Invalid or unsupported schema" << std::endl;
        return 0;
    }

    process(doc, std::ofstream(argv[2]));
}