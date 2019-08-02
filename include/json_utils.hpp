#pragma once
#include <rapidjson/document.h>

template <typename T> struct JsonSpan {
    T& wrapped;
    constexpr JsonSpan() = delete;
    constexpr explicit JsonSpan(T& ref) : wrapped(ref) {}
    auto begin() const noexcept(noexcept(wrapped.Begin())) { return wrapped.Begin(); }
    auto begin() noexcept(noexcept(wrapped.Begin())) { return wrapped.Begin(); }
    auto end() const noexcept(noexcept(wrapped.End())) { return wrapped.End(); }
    auto end() noexcept(noexcept(wrapped.End())) { return wrapped.End(); }
};

struct JsonRes : public rapidjson::Document {
  private:
    decltype(auto) json() const noexcept { return static_cast<const rapidjson::Document&>(*this); };
    decltype(auto) json() noexcept { return static_cast<rapidjson::Document&>(*this); };

  public:
    JsonRes() {
        SetObject();
        rapidjson::Value results{}, success{}, errors{}, messages{};
        results.SetArray();
        success.SetBool(false);
        errors.SetArray();
        messages.SetArray();

        AddMember("results", results, GetAllocator());
        AddMember("success", true, GetAllocator());
        AddMember("errors", errors, GetAllocator());
        AddMember("messages", messages, GetAllocator());
    }

    template <typename T>
    void result(T&& val){
        (*this)["results"].PushBack(val, GetAllocator());
    }
    template <typename T>
    void message(T&& val){
        (*this)["messages"].PushBack(val, GetAllocator());
    }
    void error(int code, std::string_view msg) {
        (*this)["success"] = false;
        rapidjson::Value err{};
        err.SetObject();
        err.AddMember("code", code, GetAllocator());
        err.AddMember("message", rapidjson::StringRef(msg.data(), msg.length()), GetAllocator());
        (*this)["errors"].PushBack(err, GetAllocator());
    };
};