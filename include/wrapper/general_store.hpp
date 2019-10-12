#pragma once
#include "handlers/async/async_store.hpp"
#include "config.hpp"

class GeneralStore {
    IniConfig m_config;
    std::string m_doc_root;

  public:
    AsyncStore async_store;

    GeneralStore() = delete;
    inline GeneralStore(IniConfig conf) : m_config(std::move(conf)), m_doc_root(m_config.http_doc_root) {}
    GeneralStore(const GeneralStore&) = delete;
    GeneralStore(GeneralStore&&) = delete;
    GeneralStore& operator=(const GeneralStore&) = delete;
    GeneralStore& operator=(GeneralStore&&) = delete;
    ~GeneralStore() = default;

    [[nodiscard]] inline const auto& config() const noexcept { return m_config; }
    [[nodiscard]] inline const auto& doc_root() const noexcept { return m_doc_root; }
};