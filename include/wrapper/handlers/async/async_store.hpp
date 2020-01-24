#pragma once
#include <chrono>
#include <cstddef>
#include <future>
#include <limits>
#include <unordered_map>
#include <virt_wrap/utility.hpp>

using namespace std::literals;

/**
 * \internal
 * Storage for asynchronous requests
 **/
struct AsyncStore {
    using IndexType = std::uint32_t;             ///< Type used as the key to #elems
    using ClockType = std::chrono::system_clock; ///< Type used for the expiration clock

    /**
     * \internal
     * Status of a task; for messaging, not storage
     * */
    enum class TaskStatus {
        non_existent, ///< task does not exist
        in_progress,  ///< task is currently being executed
        finished,     ///< task is done and its buffers are ready to be sent back
    };

    /**
     * \internal
     * Storage value type
     **/
    struct Element {
        std::chrono::time_point<ClockType> expires; ///< point of expiry of the store entry
        std::future<std::string> fut;               ///< promise of the response body
    };

    std::mutex mut{};                                                                ///< mutex to make #elems thread-safe
    std::unordered_map<IndexType, Element> elems;                                    ///< actual container of entries
    IndexType last_id{-1u};                                                          ///< last allocated id; ids are incremental
    std::chrono::seconds default_expire = 120s;                                      ///< default entry expiration time
    constexpr static std::chrono::seconds init_expire = std::chrono::seconds::max(); ///< expiration since initial request until task completion

    /**
     * \internal
     * Launches a task, optionally with a specified post-completion expiration time
     * \tparam Fcn (deduced)
     * \param[in] fcn the callable to be called to obtain the response body
     * \param[in] expire_opt an optional expiration time after task completion
     * \return the key to the newly created task in the store, or `std::nullopt` if the store is full
     **/
    template <class Fcn> std::optional<IndexType> launch(Fcn&& fcn, std::optional<std::chrono::seconds> expire_opt = std::nullopt) {
        static_assert(std::is_same_v<std::invoke_result_t<Fcn>, std::string>);
        if (elems.size() == std::numeric_limits<IndexType>::max())
            return std::nullopt;

        std::lock_guard guard{mut};
        const auto id = last_id = get_free_id();
        auto [it, sucess] = elems.emplace(id, std::move(Element{std::chrono::time_point<ClockType>{init_expire}, {}}));

        it->second.fut = std::async(std::launch::async, [&, expire_opt, id, fcn = std::forward<Fcn>(fcn)]() -> std::string {
            std::string ret = fcn(); // Not const for NRVO

            /* Housekeeping */
            std::lock_guard guard{mut};
            auto& [exp, fut /*, th*/] = elems[id];
            exp = ClockType::now() + expire_opt.value_or(default_expire);

            /// Note: when integrating deflate compression, add here as well

            return ret;
        });

        return {id};
    }

    /**
     * \internal
     * Get a task's status by key, and its response buffer if ready
     * \param[in] id the task's key
     * \return the task status and the associated buffer, which will be empty if the status is not TaskStatus::finished
     **/
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

    /**
     * \internal
     * Discard expired entries
     **/
    void gc() noexcept {
        for (auto it = elems.begin(), last = elems.end(); it != last;) {
            if (auto& [key, val] = *it; val.expires > ClockType::now() && future_status(val.fut) == std::future_status::ready) {
                it = elems.erase(it);
            } else
                ++it;
        }
    }

  private:
    /**
     * \internal
     * Get a std::future's status
     * \tparam T (deduced)
     * \param fut the future
     * \return the future's status
     **/
    template <class T> static std::future_status future_status(const std::future<T>& fut) { return fut.wait_for(std::chrono::seconds{0}); }

    /**
     * \internal
     * Get a store key that isn't in use
     * \return a currently unused key
     **/
    IndexType get_free_id() const noexcept {
        auto id = last_id;
        while (elems.count(++id) > 0)
            ;
        return id;
    }
};