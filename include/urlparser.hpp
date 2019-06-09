//
// Created by hugo on 07.06.19.
//

#pragma once

#include <algorithm>
#include <cstring>
#include <string_view>
#include <vector>
#include <ctll.hpp>
#include <ctre.hpp>
#include <flatmap.hpp>

constexpr auto target_pattern = ctll::fixed_string{R"(^($|/[^#?\s]+)?(.*?)?(#[A-Za-z_\-]+)?$)"};
static constexpr auto target_match(std::string_view sv) noexcept {
  return ctre::match<target_pattern>(sv);
}

constexpr auto target_queries = ctll::fixed_string{R"([&;]?([^=&;]*)=([^=&;]*))"};

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

class TargetParser {
protected:
  std::string_view url{};

  std::string_view path;
  flatmap<std::string_view, std::string_view> queries = {};

  constexpr std::string_view match() noexcept {
    auto res = target_match(url);
    auto [s_all, s_path, s_query, s_fragment] = res;
    path = s_path;
    return s_query;
  }
  void parse_queries(std::string_view sv_query) noexcept {
    queries.clear();
    if(sv_query.length() > 0)
      sv_query.remove_prefix(1); // strip leading '?'
    for (auto [s_match, s_name, s_value] : ctre::range<target_queries>(sv_query)){
      queries.emplace(s_name, s_value);
    }
  }

  void parse() noexcept {
    parse_queries(match());
  }
public:
  constexpr TargetParser() = default;
  explicit TargetParser(std::string_view url): url(url) {
    parse();
  }

  void setURL(const char* v) {
    url = v;
    parse();
  }
  void setURL(std::string_view sv) {
    url = sv;
    parse();
  }
  constexpr std::string_view getPath() const noexcept {
    return path;
  }

  constexpr std::string_view getURL() const noexcept {
    return url;
  }
  constexpr const flatmap<std::string_view, std::string_view>& getQueries() const noexcept {
    return queries;
  }
  constexpr std::string_view getQuery(std::string_view key) const noexcept {
    if(auto it = queries.find(key); it != queries.end())
      return it->second;
    return {};
  }
};

class URLParser : public TargetParser {
private:
  std::string_view scheme;
  std::string_view host;
  unsigned short port{0};

  void parse_all() noexcept {
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
    parse_queries(s_query);
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
    parse_all();
  }
  void setURL(std::string_view sv) {
    url = sv;
    parse_all();
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
};