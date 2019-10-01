#pragma once
#include <chrono>
#include <cstddef>
#include <future>
#include <limits>
#include <unordered_map>
#include <virt_wrap/utility.hpp>

using namespace std::literals;

struct AsyncStore {
    using IndexType = std::uint32_t;
    using ClockType = std::chrono::system_clock;

    enum class TaskStatus {
        non_existent,
        in_progress,
        finished,
    };

    struct Element {
        std::chrono::time_point<ClockType> expires;
        std::future<std::string> fut;
    };

    std::mutex mut{};
    std::unordered_map<std::uint32_t, Element> elems;
    IndexType last_id{-1u};
    std::chrono::seconds default_expire = 120s;
    constexpr static std::chrono::seconds init_expire = std::chrono::seconds::max();

    template <class Fcn> std::optional<IndexType> launch(Fcn&& fcn, std::optional<std::chrono::seconds> expire_opt = std::nullopt) {
        static_assert(std::is_same_v<std::invoke_result_t<Fcn>, std::string>);
        if (elems.size() == std::numeric_limits<IndexType>::max())
            return std::nullopt;

        std::lock_guard guard{mut};
        const auto id = last_id = get_free_id();
        auto [it, sucess] = elems.emplace(id, std::move(Element{std::chrono::time_point<ClockType>{init_expire}, {}}));

        it->second.fut = std::async(std::launch::async, [&, expire_opt, id, fcn = std::forward<Fcn>(fcn)]() -> std::string {
            std::string ret = fcn();

            /* Housekeeping */
            std::lock_guard guard{mut};
            auto& [exp, fut /*, th*/] = elems[id];
            exp = ClockType::now() + expire_opt.value_or(default_expire);

            /// Note: when integrating deflate compression, add here as well

            return ret;
        });

        return {id};
    }

    std::pair<TaskStatus, std::string> value_if_ready(IndexType id) {
        std::lock_guard guard{mut};
        const auto it = elems.find(id);
        if (it == elems.end())
            return {TaskStatus::non_existent, {}};

        auto& [expires, fut] = it->second;

        if (future_status(fut) != std::future_status::ready)
            return {TaskStatus::in_progress, {}};

        auto node = elems.extract(it);
        return {TaskStatus::finished, fut.get()};
    }

    void gc() noexcept {
        for (auto it = elems.begin(), last = elems.end(); it != last;) {
            if (auto& [key, val] = *it; val.expires > ClockType::now() && future_status(val.fut) == std::future_status::ready) {
                it = elems.erase(it);
            } else
                ++it;
        }
    }

  private:
    template <class T> static std::future_status future_status(const std::future<T>& fut) { return fut.wait_for(std::chrono::seconds{0}); }

    IndexType get_free_id() const noexcept {
        auto id = last_id;
        while (elems.count(++id) > 0)
            ;
        return id;
    }
};