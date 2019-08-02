#include <fstream>
#include <iostream>
#include <sstream>
#include <variant>
#include <flatmap.hpp>
#include <rapidxml_ns.hpp>
#include "virtxml/xmlspan.hpp"

using namespace std::literals;
using namespace rapidxml_ns;

#if 0
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


const xml_node<>& rng_deref(const xml_node<>& n, virtxml::NamedSpan<xml_node<>> definitions) {
    const auto ref_name = n.first_attribute("name")->value();
    const auto it = std::find_if(definitions.begin(), definitions.end(),
                                 [=](const xml_node<>& def) { return std::strcmp(def.first_attribute("name")->value(), ref_name) == 0; });
    return (*it).first_node();
};

struct DrefRelaxNGIt {

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
#endif

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

class DFIterator {
    const xml_node<>* n;
    auto operator++() noexcept {
        if (n->first_node()) {
            do {
                n = n->first_node();
            } while (n->first_node());
        } else if (n->next_sibling())
            n = n->next_sibling();
        else if (n->parent())
            n = n->parent();
        else
            n = nullptr;
    }
    constexpr bool operator==(const DFIterator& oth) const noexcept {

    }
};

namespace RNGAST {

class Class;
class Enum;
class Property;
class Datatype;

enum class Type { String, Bool, Int, Unsigned, ReferenceHidden };

struct Class {
    std::string name;
    std::vector<Class> classes;
    std::vector<Property> properties;
    std::optional<Datatype> dtype;
};

struct Definition : public Class {};

struct Enum {
    std::string name;
    std::vector<std::string> values;
};

struct Property {
    std::string name;
    Type type;
};

struct Datatype {
    std::string name;
    Type type;
};

constexpr std::array<std::pair<const char*, Type>, 8> builtins{{
    {"string", Type::String},
    {"unsignedInt", Type::Unsigned},
    {"unsignedLong", Type::Unsigned},
    {"positiveInteger", Type::Unsigned},
    {"octalMode", Type::String},
    {"uint8", Type::Unsigned},
    {"uint16", Type::Unsigned},
    {"uint24", Type::Unsigned},
}};

} // namespace RNGAST

void parse(const xml_node<>* gram) {
    enum class DefTag { NONE, IS_TYPE, IS_CLASSMK };

    constexpr const std::array<std::pair<std::string_view, DefTag>, 7> tags{{{"element", DefTag::IS_CLASSMK},
                                                                             {"attribute", DefTag::IS_CLASSMK},
                                                                             {"zeroOrMore", DefTag::IS_CLASSMK},
                                                                             {"oneOrMore", DefTag::IS_CLASSMK},
                                                                             {"group", DefTag::IS_CLASSMK},
                                                                             {"data", DefTag::IS_TYPE},
                                                                             {"value", DefTag::IS_TYPE}}};

    split_flatmap<std::string_view, DefTag> def_tags;

    // Tag all definitions so we later know what they induce in the AST even if the latter is incomplete
    for (auto curr = gram->first_node("define"); curr; curr = curr->next_sibling("define")) {
        auto v = curr;
        while (v->name() == "interleave"sv || v->name() == "optional"sv || v->name() == "choice"sv)
            v = v->first_node();

        const auto it = std::find_if(tags.begin(), tags.end(), [=](auto tag) { return tag.first == v->name(); });
        def_tags[curr->first_attribute("name")->value()] = it != tags.end() ? it->second : DefTag::NONE;
    }

    // ASTify each def while counting references to them
    // TODO
    for (auto curr = gram->first_node("define"); curr; curr = curr->next_sibling("define")) {
        auto v = curr;
    }

    // Rebuild full tree, removing singly referenced definitions
    // TODO
}

void process(const xml_document<>& doc, std::ofstream of) {
    const auto gram_node = doc.first_node("grammar");
    virtxml::NamedSpan<xml_node<>> definitions{"define", gram_node};

    const auto start = gram_node->first_node("start");
    parse(gram_node);
    // walk(*start, Marks::None, nullptr, definitions, of);
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