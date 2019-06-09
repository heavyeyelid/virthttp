//
// Created by hugo on 07.06.19.
//

#pragma once

#if __cplusplus > 201703L // If in C++2a or above
#  define CONSTEXPR2A constexpr
#else
#define CONSTEXPR2A
#endif

#include <algorithm>
#include <cstring>
#include <string_view>
#include <vector>
#include <ctll.hpp>
#include <ctre.hpp>

#ifndef HE_WA_CTRE_URL_SPLIT
constexpr auto url_pattern = ctll::fixed_string{R"(^(?:(http[s]?|ftp)://)?([^/]+?)(?::(\d+))?(?:$|/)([^#?\s]+)?(.*?)?(#[A-Za-z_\-]+)?$)"};

static constexpr auto url_match(std::string_view sv) noexcept {
  return ctre::match<url_pattern>(sv);
}
#else
[[deprecated]] constexpr auto url_pat_split1 = ctll::fixed_string{R"(^(?:(http[s]?|ftp)://)?([^/:]+)(.+)$)"};
[[deprecated]] constexpr auto url_pat_split2 = ctll::fixed_string{R"(^(?::(\d+))?(?:$|/)([^#?\s]+)?(.*?)?(#[A-Za-z_\-]+)?$)"};

[[deprecated]] static constexpr auto url_match_split1(std::string_view sv) noexcept {
  return ctre::match<url_pat_split1>(sv);
}
[[deprecated]] static constexpr auto url_match_split2(std::string_view sv) noexcept {
  return ctre::match<url_pat_split2>(sv);
}
#endif


class URLParser {
  struct Query {
    std::string_view name{};
    std::string_view value{};
  };

private:
  std::string url{};
  std::string_view scheme;
  std::string_view host;
  unsigned short port{0};
  std::string_view path;
  std::vector<Query> queries{};

  void parse() noexcept {
    queries.clear();
#ifndef HE_WA_CTRE_URL_SPLIT
    auto [s_all, s_scheme, s_host, s_port, s_path, s_query, s_fragment] = url_match(url);
#else
    auto [s_all1, s_scheme, s_host, s_next] = url_match_split1(url);
    auto [s_all2, s_port, s_path, s_query, s_fragment] = url_match_split2(s_next.to_view());
#endif
    scheme = s_scheme;
    host = s_host;
    if(s_port)
      port = std::strtoul(s_port.to_view().data(), nullptr, 10);
    path = s_path;
    std::string_view sv_query = s_query;

    if(sv_query.length() > 0)
      sv_query.remove_prefix(1); // strip leading '?'

    while(sv_query.length() > 0){
      auto it = std::find_if(sv_query.begin(), sv_query.end(), [](auto c){return c == '=' || c == '&' || c == ';';});
      auto& query = queries.emplace_back();
      auto pos = std::distance(sv_query.begin(), it);

      query.name = sv_query.substr(0, pos);
      sv_query = sv_query.substr(pos);

      if(it != sv_query.end() && *it == '='){
        sv_query.remove_prefix(1);
        it = std::find_if(sv_query.begin(), sv_query.end(), [](auto c){return c == '&' || c == ';';});
        pos = std::distance(sv_query.begin(), it);

        query.value = sv_query.substr(0, pos);
        sv_query = sv_query.substr(pos);

        if(it != sv_query.end())
          sv_query.remove_prefix(1);
      }
    }
    std::sort(queries.begin(), queries.end(), [](auto& a, auto& b){return a.name > b.name;});
  }

public:
  explicit URLParser(const char *uri) noexcept {
    setURL(uri);
  }
  explicit URLParser(std::string_view uri) noexcept {
    setURL(uri);
  }

  constexpr URLParser() = default;
  URLParser(const URLParser&) = default;
  URLParser(URLParser&&) noexcept = default;
  ~URLParser() noexcept = default;

  URLParser& operator=(const URLParser&) = default;
  URLParser& operator=(URLParser&&) = default;

  void setURL(const char* v) {
    url = v;
    parse();
  }
  void setURL(std::string_view sv) {
    url = sv;
    parse();
  }

  CONSTEXPR2A std::string_view getQuery(const char* query) const noexcept {
    return getQuery(std::string_view{query});
  }
  CONSTEXPR2A std::string_view getQuery(std::string_view query) const noexcept {
    Query q{query, {}};
    bool found = std::binary_search(queries.cbegin(), queries.cend(), q, [&](const Query& a, const Query& b){return a.name == b.name;});
    return found ? q.value : "";
  }

  constexpr std::string_view getScheme() const noexcept {
    return scheme;
  }

  constexpr std::string_view getHost() const noexcept {
    return host;
  }

  /**
   * Returns the port used in the URL; 0 if no explicit one
   * */
  constexpr unsigned short getPort() const noexcept {
    return port;
  }

  std::string_view getPath() const noexcept {
    return path;
  }

  const std::string& getURL() const noexcept {
      return url;
  }
};