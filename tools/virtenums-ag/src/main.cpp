#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <string_view>
#include <utility>
#include <vector>
#include <ctre.hpp>
#define FMT_HEADER_ONLY
#include <fmt/format.h>

using namespace std::literals;

constexpr static ctll::fixed_string name_pattern{".*?vir[A-Z][a-z]+([A-Za-z0-9]+);\\s*$"};
static constexpr auto name_match(std::string_view sv) noexcept { return ctre::match<name_pattern>(sv); }

constexpr static ctll::fixed_string elem_pattern{"([_A-Z]+)\\s*=.+?,([^\\n]*)"};
static constexpr auto elem_range(std::string_view sv) noexcept { return ctre::range<elem_pattern>(sv); }

int main(int argc, char** argv) {
    if (argc <= 2)
        return std::cerr << "Usage: virtenums-ag [helper-type] [file]", -1;

    const std::string_view helper = argv[1];
    const auto fname = argv[2];

    std::ifstream ifs(fname);
    if (!ifs)
        return std::cerr << "Error: unable to open file: " << fname, -1;
    std::string str((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

    const auto name_res = name_match(str);

    if (!name_res)
        return std::cerr << "Error: unnamed enum", -2;

    auto& [s_all, s_name] = name_res;
    const auto elem_res = elem_range(str);
    const auto name_w_s = s_name.to_view();
    const auto name = name_w_s.substr(0, name_w_s.size() - 1);
    fmt::print("enum class {} {{\n", name);
    std::vector<std::pair<std::string_view, std::string_view>> elems;
    auto pos = std::numeric_limits<int>::max();
    bool first = true;
    for (auto& [s_all, s_name, s_comm] : elem_res) {
        if (!first) {
            auto [a, b] = std::mismatch(elems.back().first.begin(), elems.back().first.end(), s_name.begin(), s_name.end());
            if (const auto d = std::distance(s_name.begin(), b); d < pos)
                pos = d;
        }
        elems.emplace_back(s_name, s_comm);
        first = false;
    }
    if (elems.size() == 1) {
        std::string name_ch{};
        std::transform(name.begin(), name.end(), std::back_inserter(name_ch),
                       [&](char c) { return (c >= 'A' && c <= 'Z') ? (name_ch.push_back('_'), c) : std::toupper(c); });
        name_ch = name_ch.substr(1);
        const auto idx_first_underscore = elems[0].first.find('_');
        const auto idx_sec_underscore = elems[0].first.find('_', idx_first_underscore + 1);
        const auto [it_name, it_el] = std::mismatch(name_ch.begin(), name_ch.end(), elems[0].first.begin() + idx_sec_underscore + 1, elems[0].first.end());
        pos = std::distance(name_ch.begin(), it_name);
    }
    for (auto& [name, comm] : elems) {
        auto full_name = name;
        name.remove_prefix(pos);
        std::cout << '\t' << name << " = " << full_name << ',' << comm << std::endl;
    }
    std::cout << "};\n";

    fmt::print("class {0}sC : public {1}<{0}sC, {0}> {{\n", name, helper);
    fmt::print("\tusing Base =  {1}<{0}sC, {0}>;\n\tfriend Base;\n", name, helper);
    std::cout << "\tconstexpr static std::array values = {" << std::flush;
    auto i = elems.size() - 1;
    for (const auto& [name, comm] : elems) {
        std::string s = {name.data(), name.size()};
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
        fmt::print("\"{}\"{}", s, i != 0 ? ", " : ""), --i;
    }

    fmt::print("}};\n}} constexpr static {}s{{}};\n", name);

    if (helper == "EnumSetHelper"sv) {
        fmt::print("[[nodiscard]] constexpr {0} operator|({0} lhs, {0} rhs) noexcept {{ return {0}{{to_integral(lhs) | to_integral(rhs)}};}}\n",
                   name);
        fmt::print("constexpr {0}& operator|=({0}& lhs, {0} rhs) noexcept {{ return lhs = {0}{{to_integral(lhs) | to_integral(rhs)}};}}\n", name);
    }
    return 0;
}