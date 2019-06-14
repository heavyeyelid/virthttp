//
// Created by _as on 2019-06-09.
//

#ifndef HEAVYEYELID_FLATMAP_HPP
#define HEAVYEYELID_FLATMAP_HPP

#include <algorithm>
#include <new>
#include <tuple>
#include <type_traits>
#include <vector>
#include <experimental/type_traits>

namespace type_traits {
template <typename T, typename U> using equal_compare_t = decltype(std::declval<const T&>() == std::declval<const U&>());

template <typename T, typename U> using are_equal_comparable = std::experimental::is_detected<equal_compare_t, T, U>;

template <typename C, typename T, typename U> using is_callable_t = decltype(std::declval<C&>()(std::declval<T>(), std::declval<U>()));

template <typename C, typename T, typename U> using is_callable = std::experimental::is_detected<is_callable_t, C, T, U>;

template <typename I>
using is_input_iterator = typename std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<I>::iterator_category>::type;
} // namespace type_traits

namespace impl {
template <typename Key, typename Value> class flatmap_storage {
    using storage = std::vector<std::pair<Key, Value>>;

  public:
    using value_type = std::pair<const Key, Value>;
    using size_type = typename storage::size_type;

    flatmap_storage() = default;

    template <typename type, typename container_iterator> class iterator_type;
    using iterator = iterator_type<value_type, typename storage::iterator>;
    using const_iterator = iterator_type<const value_type, typename storage::const_iterator>;

    void clear() noexcept { m_values.clear(); }
    bool empty() const noexcept { return m_values.empty(); }
    size_type size() const noexcept { return m_values.size(); }

    iterator begin() noexcept { return iterator{std::begin(m_values)}; }
    iterator end() noexcept { return iterator{std::end(m_values)}; }
    const_iterator begin() const noexcept { return const_iterator{std::begin(m_values)}; }
    const_iterator end() const noexcept { return const_iterator{std::end(m_values)}; }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }

  protected:
    template <typename V, typename I> static auto inner(iterator_type<V, I> i) { return i.i; }

    storage m_values;
};

template <typename Key, typename Value> class split_flatmap_storage {
    using key_storage = std::vector<Key>;
    using value_storage = std::vector<Value>;

  public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<Key, Value>;
    using reference = std::pair<const Key&, Value&>&;
    using pointer = std::pair<const Key&, Value&>*;
    using size_type = typename key_storage::size_type;

    split_flatmap_storage() = default;

    template <typename container> class iterator_type;
    using iterator = iterator_type<split_flatmap_storage>;
    using const_iterator = iterator_type<const split_flatmap_storage>;

    void clear() noexcept {
        m_keys.clear();
        m_values.clear();
    }
    bool empty() const noexcept { return m_values.empty(); }
    size_type size() const noexcept { return m_values.size(); }

    iterator begin() noexcept { return iterator{*this, 0}; }
    iterator end() noexcept { return iterator{*this, m_keys.size()}; }
    const_iterator begin() const noexcept { return const_iterator{*this, 0}; }
    const_iterator end() const noexcept { return const_iterator{*this, m_keys.size()}; }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }

  protected:
    key_storage m_keys;
    value_storage m_values;
};
} // namespace impl
template <typename Key, typename Value> class unordered_flatmap : private impl::flatmap_storage<Key, Value> {
    static_assert(std::is_nothrow_move_constructible<Key>{});
    static_assert(std::is_nothrow_move_constructible<Value>{});

  public:
    using value_type = typename impl::flatmap_storage<Key, Value>::value_type;
    using iterator = typename impl::flatmap_storage<Key, Value>::iterator;
    using const_iterator = typename impl::flatmap_storage<Key, Value>::const_iterator;
    using size_type = typename impl::flatmap_storage<Key, Value>::size_type;

    unordered_flatmap() = default;
    unordered_flatmap(std::initializer_list<value_type> list);
    template <typename Iterator, typename EIterator,
              typename = std::enable_if_t<type_traits::is_input_iterator<Iterator>{} && type_traits::are_equal_comparable<Iterator, EIterator>{} &&
                                          std::is_constructible<value_type, typename std::iterator_traits<Iterator>::value_type>{}>>
    unordered_flatmap(Iterator b, EIterator e);
    using impl::flatmap_storage<Key, Value>::clear;
    using impl::flatmap_storage<Key, Value>::empty;
    using impl::flatmap_storage<Key, Value>::size;
    using impl::flatmap_storage<Key, Value>::begin;
    using impl::flatmap_storage<Key, Value>::end;
    using impl::flatmap_storage<Key, Value>::cbegin;
    using impl::flatmap_storage<Key, Value>::cend;
    template <typename K, typename = std::enable_if_t<type_traits::are_equal_comparable<Key, const K&>{}>>
    size_type count(const K& key) const noexcept {
        return find(key) == end() ? 0 : 1;
    }
    std::pair<iterator, bool> insert(const value_type& v);
    std::pair<iterator, bool> insert(value_type&& v);
    template <typename K, typename V,
              typename = std::enable_if_t<type_traits::are_equal_comparable<Key, K>{} && std::is_constructible<Value, V>{} &&
                                          std::is_assignable<Value, V>{}>>
    std::pair<iterator, bool> insert_or_assign(K&& k, V&& v);
    template <typename... T, typename = std::enable_if_t<std::is_constructible<value_type, T...>{}>> std::pair<iterator, bool> emplace(T&&... t);
    template <typename K, typename... V,
              typename = std::enable_if_t<type_traits::are_equal_comparable<Key, K>{} && std::is_constructible<Value, V...>{}>>
    std::pair<iterator, bool> try_emplace(K&& key, V&&... v);
    void erase(iterator i);
    template <typename K, typename = std::enable_if_t<type_traits::are_equal_comparable<Key, const K&>{}>> size_type erase(const K& key);
    template <typename T, typename = std::enable_if_t<type_traits::are_equal_comparable<Key, T>{}>> Value& operator[](const T& key);
    template <typename T, typename = std::enable_if_t<type_traits::are_equal_comparable<Key, T>{}>> iterator find(const T& key) noexcept;
    template <typename T, typename = std::enable_if_t<type_traits::are_equal_comparable<Key, T>{}>> const_iterator find(const T& key) const noexcept;
};

template <typename Key, typename Value> class unordered_split_flatmap : private impl::split_flatmap_storage<Key, Value> {
    static_assert(std::is_nothrow_move_constructible<Key>{});
    static_assert(std::is_nothrow_move_constructible<Value>{});

  public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = typename impl::split_flatmap_storage<Key, Value>::value_type;
    using reference = typename impl::split_flatmap_storage<Key, Value>::reference;
    using pointer = typename impl::split_flatmap_storage<Key, Value>::pointer;
    using iterator = typename impl::split_flatmap_storage<Key, Value>::iterator;
    using const_iterator = typename impl::split_flatmap_storage<Key, Value>::const_iterator;
    using size_type = typename impl::split_flatmap_storage<Key, Value>::size_type;

    unordered_split_flatmap() = default;
    unordered_split_flatmap(std::initializer_list<value_type> list);
    template <typename Iterator, typename EIterator,
              typename = std::enable_if_t<type_traits::is_input_iterator<Iterator>{} && type_traits::are_equal_comparable<Iterator, EIterator>{} &&
                                          std::is_constructible<value_type, typename std::iterator_traits<Iterator>::value_type>{}>>
    unordered_split_flatmap(Iterator b, EIterator e);
    using impl::split_flatmap_storage<Key, Value>::clear;
    using impl::split_flatmap_storage<Key, Value>::empty;
    using impl::split_flatmap_storage<Key, Value>::size;
    using impl::split_flatmap_storage<Key, Value>::begin;
    using impl::split_flatmap_storage<Key, Value>::end;
    using impl::split_flatmap_storage<Key, Value>::cbegin;
    using impl::split_flatmap_storage<Key, Value>::cend;
    template <typename K, typename = std::enable_if_t<type_traits::are_equal_comparable<Key, const K&>{}>>
    size_type count(const K& key) const noexcept {
        return find(key) == end() ? 0 : 1;
    }
    std::pair<iterator, bool> insert(const value_type& v);
    std::pair<iterator, bool> insert(value_type&& v);
    template <typename K, typename V,
              typename = std::enable_if_t<type_traits::are_equal_comparable<Key, K>{} && std::is_constructible<Value, V>{} &&
                                          std::is_assignable<Value, V>{}>>
    std::pair<iterator, bool> insert_or_assign(K&& k, V&& v);
    template <typename... T, typename = std::enable_if_t<std::is_constructible<value_type, T...>{}>> std::pair<iterator, bool> emplace(T&&... t);
    template <typename K, typename... V,
              typename = std::enable_if_t<type_traits::are_equal_comparable<Key, K>{} && std::is_constructible<Value, V...>{}>>
    std::pair<iterator, bool> try_emplace(K&& key, V&&... v);
    void erase(iterator i);
    template <typename K, typename = std::enable_if_t<type_traits::are_equal_comparable<Key, const K&>{}>> size_type erase(const K& key);
    template <typename T, typename = std::enable_if_t<type_traits::are_equal_comparable<Key, T>{}>> Value& operator[](const T& key);
    template <typename T, typename = std::enable_if_t<type_traits::are_equal_comparable<Key, T>{}>> iterator find(const T& key) noexcept;
    template <typename T, typename = std::enable_if_t<type_traits::are_equal_comparable<Key, T>{}>> const_iterator find(const T& key) const noexcept;
};

template <typename Key, typename Value, typename Compare = std::less<>>
class split_flatmap : private impl::split_flatmap_storage<Key, Value>, private Compare {
    static_assert(std::is_nothrow_move_constructible<Key>{});
    static_assert(std::is_nothrow_move_constructible<Value>{});

  public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = typename impl::split_flatmap_storage<Key, Value>::value_type;
    using reference = typename impl::split_flatmap_storage<Key, Value>::reference;
    using pointer = typename impl::split_flatmap_storage<Key, Value>::pointer;
    using iterator = typename impl::split_flatmap_storage<Key, Value>::iterator;
    using const_iterator = typename impl::split_flatmap_storage<Key, Value>::const_iterator;
    using size_type = typename impl::split_flatmap_storage<Key, Value>::size_type;

    split_flatmap() = default;
    split_flatmap(std::initializer_list<value_type> list);
    template <typename Iterator, typename EIterator,
              typename = std::enable_if_t<type_traits::is_input_iterator<Iterator>{} && type_traits::are_equal_comparable<Iterator, EIterator>{} &&
                                          std::is_constructible<value_type, typename std::iterator_traits<Iterator>::value_type>{}>>
    split_flatmap(Iterator b, EIterator e);
    using impl::split_flatmap_storage<Key, Value>::clear;
    using impl::split_flatmap_storage<Key, Value>::empty;
    using impl::split_flatmap_storage<Key, Value>::size;
    using impl::split_flatmap_storage<Key, Value>::begin;
    using impl::split_flatmap_storage<Key, Value>::end;
    using impl::split_flatmap_storage<Key, Value>::cbegin;
    using impl::split_flatmap_storage<Key, Value>::cend;
    template <typename K, typename = std::enable_if_t<type_traits::are_equal_comparable<Key, const K&>{}>>
    size_type count(const K& key) const noexcept {
        return find(key) == end() ? 0 : 1;
    }
    std::pair<iterator, bool> insert(const value_type& v);
    std::pair<iterator, bool> insert(value_type&& v);
    template <typename K, typename V,
              typename = std::enable_if_t<type_traits::is_callable<Compare, Key, K>{} && std::is_constructible<Value, V>{} &&
                                          std::is_assignable<Value, V>{}>>
    std::pair<iterator, bool> insert_or_assign(K&& k, V&& v);
    template <typename... T, typename = std::enable_if_t<std::is_constructible<value_type, T...>{}>> std::pair<iterator, bool> emplace(T&&... t);
    template <typename K, typename... V,
              typename = std::enable_if_t<type_traits::is_callable<Compare, Key, K>{} && std::is_constructible<Value, V...>{}>>
    std::pair<iterator, bool> try_emplace(K&& key, V&&... v);
    void erase(iterator i);
    template <typename K, typename = std::enable_if_t<type_traits::is_callable<Compare, K, Key>{}>> size_type erase(const K& key);
    template <typename T, typename = std::enable_if_t<type_traits::is_callable<Compare, Key, T>{}>> Value& operator[](const T& key);
    template <typename T, typename = std::enable_if_t<type_traits::is_callable<Compare, Key, T>{}>> iterator find(const T& key) noexcept;
    template <typename T, typename = std::enable_if_t<type_traits::is_callable<Compare, Key, T>{}>> const_iterator find(const T& key) const noexcept;

  private:
    auto key_iter(iterator i) {
        using d = typename std::iterator_traits<iterator>::difference_type;
        return this->m_keys.begin() + static_cast<d>(index(i));
    }
    auto value_iter(iterator i) {
        using d = typename std::iterator_traits<iterator>::difference_type;
        return this->m_values.begin() + static_cast<d>(index(i));
    }
    static const Key& key_of(const Key& k) { return k; }
    static const Key& key_of(const value_type& v) { return v.first; }

    template <typename T> std::pair<iterator, bool> find_key(const T& t) noexcept;
    template <typename T> std::pair<const_iterator, bool> find_key(const T& t) const noexcept;
};

template <typename Key, typename Value, typename Compare>
template <typename Iterator, typename EIterator, typename>
split_flatmap<Key, Value, Compare>::split_flatmap(Iterator b, EIterator e) {
    while (b != e) {
        insert(*b);
        ++b;
    }
}

template <typename Key, typename Value, typename Compare> split_flatmap<Key, Value, Compare>::split_flatmap(std::initializer_list<value_type> list) {
    for (auto& x : list) {
        insert(x);
    }
}

template <typename Key, typename Value, typename Compare>
auto split_flatmap<Key, Value, Compare>::insert(const value_type& v) -> std::pair<iterator, bool> {
    if (auto [iter, exact_match] = find_key(v.first); exact_match) {
        return {iter, false};
    } else {
        auto ki = key_iter(iter);
        auto vi = value_iter(iter);
        auto& vals = this->m_values;
        auto& keys = this->m_keys;
        vals.insert(vi, v.second);
        keys.insert(ki, v.first);
        return {{*this, index(iter)}, true};
    }
}

template <typename Key, typename Value, typename Compare>
auto split_flatmap<Key, Value, Compare>::insert(value_type&& v) -> std::pair<iterator, bool> {
    if (auto [iter, exact_match] = find_key(v.first); exact_match) {
        return {iter, false};
    } else {
        this->m_keys.insert(key_iter(iter), v.first);
        this->m_values.insert(value_iter(iter), std::move(v.second));
        return {{*this, index(iter)}, true};
    }
}

template <typename Key, typename Value, typename Compare>
template <typename... T, typename>
auto split_flatmap<Key, Value, Compare>::emplace(T&&... t) -> std::pair<iterator, bool> {
    return insert(value_type(std::forward<T>(t)...));
}
template <typename Key, typename Value, typename Compare>
template <typename K, typename V, typename>
auto split_flatmap<Key, Value, Compare>::insert_or_assign(K&& k, V&& v) -> std::pair<iterator, bool> {
    if (auto [iter, exact_match] = find_key(k); exact_match) {
        iter->second = std::forward<V>(v);
        return {iter, false};
    } else {
        this->m_keys.emplace(key_iter(iter), std::forward<K>(k));
        this->m_values.emplace(value_iter(iter), std::forward<V>(v));
        return {{*this, index(iter)}, true};
    }
}
template <typename Key, typename Value, typename Compare>
template <typename K, typename... V, typename>
auto split_flatmap<Key, Value, Compare>::try_emplace(K&& key, V&&... v) -> std::pair<iterator, bool> {
    if (auto [iter, exact_match] = find_key(key); exact_match) {
        return {iter, false};
    } else {
        this->m_keys.emplace(key_iter(iter), std::forward<K>(key));
        this->m_values.emplace(value_iter(iter), std::forward<V>(v)...);
        return {{*this, index(iter)}, true};
    }
}
template <typename Key, typename Value, typename Compare>
template <typename K, typename>
auto split_flatmap<Key, Value, Compare>::operator[](const K& key) -> Value& {
    if (auto [iter, exact_match] = find_key(key); exact_match) {
        return iter->second;
    } else {
        this->m_keys.insert(key_iter(iter), key);
        auto i = this->m_values.insert(value_iter(iter), Value());
        return *i;
    }
}

template <typename Key, typename Value, typename Compare> void split_flatmap<Key, Value, Compare>::erase(iterator i) {
    this->m_values.erase(value_iter(i));
    this->m_keys.erase(key_iter(i));
}

template <typename Key, typename Value, typename Compare>
template <typename K, typename>
auto split_flatmap<Key, Value, Compare>::erase(const K& k) -> size_type {
    if (auto [iter, exact_match] = find_key(k); exact_match) {
        erase(iter);
        return 1;
    }
    return 0;
};

template <typename Key, typename Value, typename Compare>
template <typename T>
auto split_flatmap<Key, Value, Compare>::find_key(const T& t) noexcept -> std::pair<iterator, bool> {
    Compare& comp = *this;
    auto key_compare = [&comp, this](auto& lh, auto& rh) { return comp(this->key_of(lh), this->key_of(rh)); };
    auto i = std::lower_bound(std::begin(this->m_keys), std::end(this->m_keys), t, key_compare);
    auto d = std::distance(std::begin(this->m_keys), i);
    if (i != std::end(this->m_keys) && !key_compare(t, *i)) {
        return {iterator{*this, static_cast<size_type>(d)}, true};
    }
    return {iterator{*this, static_cast<size_type>(d)}, false};
}

template <typename Key, typename Value, typename Compare>
template <typename T>
auto split_flatmap<Key, Value, Compare>::find_key(const T& t) const noexcept -> std::pair<const_iterator, bool> {
    const Compare& comp = *this;
    auto key_compare = [&comp, this](auto& lh, auto& rh) { return comp(this->key_of(lh), this->key_of(rh)); };
    auto i = std::lower_bound(std::begin(this->m_keys), std::end(this->m_keys), t, key_compare);
    auto d = static_cast<size_type>(std::distance(std::begin(this->m_keys), i));
    if (i != std::end(this->m_keys) && !key_compare(t, *i)) {
        return {const_iterator{*this, d}, true};
    }
    return {const_iterator{*this, d}, false};
}

template <typename Key, typename Value, typename Compare>
template <typename K, typename>
auto split_flatmap<Key, Value, Compare>::find(const K& key) noexcept -> iterator {
    if (auto [iter, exact_match] = find_key(key); exact_match) {
        return iter;
    }
    return end();
}

template <typename Key, typename Value, typename Compare>
template <typename K, typename>
auto split_flatmap<Key, Value, Compare>::find(const K& key) const noexcept -> const_iterator {
    if (auto [iter, exact_match] = find_key(key); exact_match) {
        return iter;
    }
    return end();
}

template <typename Key, typename Value>
template <typename K, typename... V, typename>
inline auto unordered_split_flatmap<Key, Value>::try_emplace(K&& key, V&&... v) -> std::pair<iterator, bool> {
    if (auto i = find(key); i != end()) {
        return {i, false};
    }
    return emplace(std::forward<K>(key), std::forward<V>(v)...);
}

template <typename Key, typename Value>
template <typename K, typename V, typename>
inline auto unordered_split_flatmap<Key, Value>::insert_or_assign(K&& k, V&& v) -> std::pair<iterator, bool> {
    if (auto i = find(k); i != end()) {
        i->second = std::forward<V>(v);
        return {i, false};
    }
    return emplace(std::forward<K>(k), std::forward<V>(v));
}

template <typename Key, typename Value>
template <typename Iterator, typename EIterator, typename>
inline unordered_split_flatmap<Key, Value>::unordered_split_flatmap(Iterator b, EIterator e) {
    while (b != e) {
        emplace(*b);
        ++b;
    }
}

template <typename Key, typename Value> inline unordered_split_flatmap<Key, Value>::unordered_split_flatmap(std::initializer_list<value_type> list) {
    for (auto&& x : list) {
        emplace(std::move(x));
    }
};

template <typename Key, typename Value>
template <typename K, typename>
inline auto unordered_split_flatmap<Key, Value>::erase(const K& key) -> size_type {
    if (auto i = find(key); i != end()) {
        erase(i);
        return 1;
    }
    return 0;
};

template <typename Key, typename Value> inline void unordered_split_flatmap<Key, Value>::erase(iterator i) {
    if (i != std::prev(end())) {
        auto kp = std::addressof(i->first);
        auto vp = std::addressof(i->second);
        kp->~Key();
        vp->~Value();
        new (const_cast<Key*>(kp)) Key(std::move(this->m_keys.back()));
        new (vp) Value(std::move(this->m_values.back()));
    }
    this->m_keys.pop_back();
    this->m_values.pop_back();
}

template <typename Key, typename Value>
template <typename... T, typename>
inline auto unordered_split_flatmap<Key, Value>::emplace(T&&... t) -> std::pair<iterator, bool> {
    return insert(value_type(std::forward<T>(t)...));
}

template <typename Key, typename Value>
template <typename T, typename>
inline auto unordered_split_flatmap<Key, Value>::operator[](const T& key) -> Value& {
    if (auto i = find(key); i != end()) {
        return i->second;
    }
    emplace(key, Value());
    return this->m_values.back();
}

template <typename Key, typename Value>
template <typename T, typename>
inline auto unordered_split_flatmap<Key, Value>::find(const T& key) noexcept -> iterator {
    auto ki = std::find_if(std::begin(this->m_keys), std::end(this->m_keys), [&](auto& x) { return x == key; });
    auto d = static_cast<size_type>(std::distance(std::begin(this->m_keys), ki));
    return iterator{*this, d};
}

template <typename Key, typename Value>
template <typename T, typename>
inline auto unordered_split_flatmap<Key, Value>::find(const T& key) const noexcept -> const_iterator {
    auto ki = std::find_if(std::begin(this->m_keys), std::end(this->m_keys), [&](auto& x) { return x == key; });
    auto d = static_cast<size_type>(std::distance(std::begin(this->m_keys), ki));
    return const_iterator{*this, d};
}

template <typename Key, typename Value> inline auto unordered_split_flatmap<Key, Value>::insert(const value_type& v) -> std::pair<iterator, bool> {
    if (auto i = find(v.first); i != end()) {
        return {i, false};
    }
    this->m_keys.push_back(v.first);
    this->m_values.push_back(v.second);
    return {{std::prev(this->m_keys.end()), std::prev(this->m_values.end())}, true};
}

template <typename Key, typename Value> inline auto unordered_split_flatmap<Key, Value>::insert(value_type&& v) -> std::pair<iterator, bool> {
    if (auto i = find(v.first); i != end()) {
        return {i, false};
    }
    this->m_keys.push_back(std::move(v.first));
    this->m_values.push_back(std::move(v.second));
    return {std::prev(end()), true};
}

template <typename Key, typename Value, typename Compare = std::less<>> class flatmap : private impl::flatmap_storage<Key, Value>, private Compare {
    static_assert(std::is_nothrow_move_constructible<Key>{});
    static_assert(std::is_nothrow_move_constructible<Value>{});
    using storage = std::vector<std::pair<Key, Value>>;

  public:
    using value_type = typename impl::flatmap_storage<Key, Value>::value_type;
    using iterator = typename impl::flatmap_storage<Key, Value>::iterator;
    using const_iterator = typename impl::flatmap_storage<Key, Value>::const_iterator;
    using size_type = typename impl::flatmap_storage<Key, Value>::size_type;

    flatmap() = default;
    flatmap(std::initializer_list<value_type> list);
    template <typename Iterator, typename EIterator,
              typename = std::enable_if_t<type_traits::is_input_iterator<Iterator>{} && type_traits::are_equal_comparable<Iterator, EIterator>{} &&
                                          std::is_constructible<value_type, typename std::iterator_traits<Iterator>::value_type>{}>>
    flatmap(Iterator b, EIterator e);
    using impl::flatmap_storage<Key, Value>::clear;
    using impl::flatmap_storage<Key, Value>::empty;
    using impl::flatmap_storage<Key, Value>::size;
    using impl::flatmap_storage<Key, Value>::begin;
    using impl::flatmap_storage<Key, Value>::end;
    using impl::flatmap_storage<Key, Value>::cbegin;
    using impl::flatmap_storage<Key, Value>::cend;

    template <typename T, typename = std::enable_if_t<type_traits::is_callable<Compare, T, Key>{}>> iterator find(const T& key) noexcept;
    template <typename T, typename = std::enable_if_t<type_traits::is_callable<Compare, T, Key>{}>> const_iterator find(const T& key) const noexcept;
    template <typename T, typename = std::enable_if_t<type_traits::is_callable<Compare, T, Key>{}>> Value& operator[](const T& key);
    std::pair<iterator, bool> insert(const value_type& v);
    std::pair<iterator, bool> insert(value_type&& v);
    template <typename K, typename V,
              typename = std::enable_if_t<type_traits::is_callable<Compare, K, Key>{} && std::is_constructible<Value, V>{} &&
                                          std::is_assignable<Value, V>{}>>
    std::pair<iterator, bool> insert_or_assign(K&& key, V&& value);
    template <typename... T, typename = std::enable_if_t<std::is_constructible<value_type, T...>{}>> std::pair<iterator, bool> emplace(T&&... t);
    template <typename K, typename... V,
              typename = std::enable_if_t<type_traits::is_callable<Compare, K, Key>{} && std::is_constructible<Value, V...>{}>>
    std::pair<iterator, bool> try_emplace(K&& key, V&&... v);
    void erase(iterator i);
    template <typename K, typename = std::enable_if_t<type_traits::is_callable<Compare, K, Key>{}>> size_type erase(const K& key);
    template <typename K, typename = std::enable_if_t<type_traits::is_callable<Compare, K, Key>{}>> size_type count(const K& key) const noexcept;

  private:
    using impl::flatmap_storage<Key, Value>::inner;
    static const Key& key_of(const Key& k) { return k; }
    static const Key& key_of(const value_type& v) { return v.first; }
    template <typename T> std::pair<iterator, bool> find_key(const T& key) noexcept;
    template <typename T> std::pair<const_iterator, bool> find_key(const T& key) const noexcept;
};

template <typename Key, typename Value, typename Compare> inline flatmap<Key, Value, Compare>::flatmap(std::initializer_list<value_type> list) {
    this->m_values.reserve(list.size());
    for (auto&& x : list) {
        insert(x);
    }
}

template <typename Key, typename Value, typename Compare>
template <typename Iterator, typename EIterator, typename>
inline flatmap<Key, Value, Compare>::flatmap(Iterator b, EIterator e) {
    while (b != e) {
        emplace(*b);
        ++b;
    }
}

template <typename Key, typename Value, typename Compare>
template <typename K>
inline auto flatmap<Key, Value, Compare>::find_key(const K& key) noexcept -> std::pair<iterator, bool> {
    Compare& comp = *this;
    auto key_compare = [&comp, this](auto& lh, auto& rh) { return comp(key_of(lh), key_of(rh)); };
    auto i = std::lower_bound(begin(), end(), key, key_compare);
    if (i != end() && !key_compare(key, *i)) {
        return {i, true};
    }
    return {i, false};
}

template <typename Key, typename Value, typename Compare>
template <typename K>
inline auto flatmap<Key, Value, Compare>::find_key(const K& key) const noexcept -> std::pair<const_iterator, bool> {
    const Compare& comp = *this;
    auto key_compare = [&comp, this](auto& lh, auto& rh) { return comp(key_of(lh), key_of(rh)); };
    auto i = std::lower_bound(begin(), end(), key, key_compare);
    if (i != end() && !key_compare(key, *i)) {
        return {i, true};
    }
    return {i, false};
}
template <typename Key, typename Value, typename Compare>
template <typename T, typename>
inline auto flatmap<Key, Value, Compare>::operator[](const T& key) -> Value& {
    auto [iter, exact_match] = find_key(key);
    if (exact_match)
        return iter->second;
    auto i = this->m_values.insert(inner(iter), {key, Value{}});
    return i->second;
}

template <typename Key, typename Value, typename Compare>
inline auto flatmap<Key, Value, Compare>::insert(const value_type& v) -> std::pair<iterator, bool> {
    auto [iter, exact_match] = find_key(v);
    if (exact_match)
        return {iter, false};
    if (iter == end()) {
        this->m_values.push_back(v);
        return {std::prev(end()), true};
    }
    iterator i{this->m_values.insert(inner(iter), v)};
    return {i, true};
}
template <typename Key, typename Value, typename Compare>
inline auto flatmap<Key, Value, Compare>::insert(value_type&& v) -> std::pair<iterator, bool> {
    auto [iter, exact_match] = find_key(v);
    if (exact_match)
        return {iter, false};
    if (iter == end()) {
        this->m_values.push_back(std::move(v));
        return {std::prev(end()), true};
    }
    iterator i{this->m_values.insert(inner(iter), std::move(v))};
    return {i, true};
}

template <typename Key, typename Value, typename Compare>
template <typename K, typename V, typename>
inline auto flatmap<Key, Value, Compare>::insert_or_assign(K&& key, V&& value) -> std::pair<iterator, bool> {
    auto [iter, exact_match] = find_key(key);
    if (exact_match) {
        iter->second = std::forward<V>(value);
        return {iter, false};
    }
    auto i = this->m_values.emplace(inner(iter), std::forward<K>(key), std::forward<V>(value));
    return {iterator{i}, true};
}

template <typename Key, typename Value, typename Compare>
template <typename... T, typename>
inline auto flatmap<Key, Value, Compare>::emplace(T&&... t) -> std::pair<iterator, bool> {
    std::pair<Key, Value> v(std::forward<T>(t)...);
    auto [iter, exact_match] = find_key(v.first);
    if (exact_match) {
        return {iter, false};
    }
    auto i = this->m_values.emplace(inner(iter), std::move(v));
    return {iterator{i}, true};
}

template <typename Key, typename Value, typename Compare>
template <typename K, typename... V, typename>
inline auto flatmap<Key, Value, Compare>::try_emplace(K&& key, V&&... v) -> std::pair<iterator, bool> {
    auto [iter, exact_match] = find_key(key);
    if (exact_match) {
        return {iter, false};
    }
    auto i = this->m_values.emplace(inner(iter), std::forward<K>(key), Value(std::forward<V>(v)...));
    return {iterator{i}, true};
}

template <typename Key, typename Value, typename Compare>
template <typename K, typename>
inline auto flatmap<Key, Value, Compare>::find(const K& key) noexcept -> iterator {
    auto [iter, exact_match] = find_key(key);
    return exact_match ? iter : end();
}

template <typename Key, typename Value, typename Compare>
template <typename K, typename>
inline auto flatmap<Key, Value, Compare>::find(const K& key) const noexcept -> const_iterator {
    auto [iter, exact_match] = find_key(key);
    return exact_match ? iter : end();
}

template <typename Key, typename Value, typename Compare> inline void flatmap<Key, Value, Compare>::erase(iterator i) {
    this->m_values.erase(inner(i));
}

template <typename Key, typename Value, typename Compare>
template <typename T, typename>
inline auto flatmap<Key, Value, Compare>::erase(const T& key) -> size_type {
    auto [iter, exact_match] = find_key(key);
    if (!exact_match)
        return 0;
    erase(iter);
    return 1;
}

template <typename Key, typename Value, typename Compare>
template <typename T, typename>
inline auto flatmap<Key, Value, Compare>::count(const T& key) const noexcept -> size_type {
    auto [iter, exact_match] = find_key(key);
    return exact_match ? 1 : 0;
}

template <typename Key, typename Value> template <typename type, typename container_iterator> class impl::flatmap_storage<Key, Value>::iterator_type {
    template <typename V, typename I> friend class impl::flatmap_storage<Key, Value>::iterator_type;
    friend class impl::flatmap_storage<Key, Value>;

  public:
    using value_type = type;
    using iterator_category = std::bidirectional_iterator_tag;
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = typename storage::iterator::difference_type;

    constexpr iterator_type() noexcept = default;
    explicit constexpr iterator_type(container_iterator i_) noexcept : i{i_} {}
    constexpr iterator_type& operator++() noexcept {
        ++i;
        return *this;
    }
    constexpr iterator_type operator++(int) noexcept {
        auto rv = *this;
        ++i;
        return rv;
    }
    constexpr iterator_type& operator--() noexcept {
        --i;
        return *this;
    }
    constexpr iterator_type operator--(int) noexcept {
        auto rv = *this;
        --i;
        return rv;
    }
    constexpr reference operator*() const noexcept { return reinterpret_cast<reference>(*i); }
    constexpr pointer operator->() const noexcept { return &operator*(); }
    template <typename V, typename I, typename = std::enable_if_t<type_traits::are_equal_comparable<container_iterator, I>{}>>
    inline constexpr bool operator==(const iterator_type<V, I>& ci) const noexcept {
        return i == ci.i;
    }
    template <typename V, typename I, typename = std::enable_if_t<type_traits::are_equal_comparable<container_iterator, I>{}>>
    inline constexpr bool operator!=(const iterator_type<V, I>& ci) const noexcept {
        return i != ci.i;
    }

  private:
    container_iterator i;
};

template <typename Key, typename Value> template <typename container> class impl::split_flatmap_storage<Key, Value>::iterator_type {
    template <typename C> friend class impl::split_flatmap_storage<Key, Value>::iterator_type;
    friend class impl::split_flatmap_storage<Key, Value>;
    using data = std::pair<const typename container::key_type&, typename container::mapped_type&>;

  public:
    class data_ptr {
      public:
        template <typename T, typename U> data_ptr(T& t, U& u) : m{t, u} {}
        data* operator->() { return &m; }

      private:
        data m;
    };
    using value_type = typename container::value_type;
    using iterator_category = std::bidirectional_iterator_tag;
    using reference = data;
    using pointer = data_ptr;
    using difference_type = typename key_storage::iterator::difference_type;

    constexpr iterator_type() noexcept = default;
    constexpr iterator_type(container& c_, typename container::size_type idx_) noexcept : c{&c_}, idx{idx_} {}
    constexpr iterator_type& operator++() noexcept {
        ++idx;
        return *this;
    }
    constexpr iterator_type operator++(int) noexcept {
        auto rv = *this;
        operator++();
        return rv;
    }
    constexpr iterator_type& operator--() noexcept {
        --idx;
        return *this;
    }
    constexpr iterator_type operator--(int) noexcept {
        auto rv = *this;
        operator--();
        return rv;
    }
    constexpr reference operator*() const noexcept { return {c->m_keys[idx], c->m_values[idx]}; }
    constexpr pointer operator->() const noexcept { return {c->m_keys[idx], c->m_values[idx]}; }
    template <typename C> constexpr bool operator==(const iterator_type<C>& ci) const noexcept { return c == ci.c && idx == ci.idx; }
    template <typename C> constexpr bool operator!=(const iterator_type<C>& ci) const noexcept { return !(*this == ci); }
    friend auto index(iterator_type i) noexcept { return i.idx; }

  private:
    container* c;
    typename container::size_type idx;
};

template <typename Key, typename Value> inline unordered_flatmap<Key, Value>::unordered_flatmap(std::initializer_list<value_type> list) {
    this->m_values.reserve(list.size());
    for (auto&& x : list) {
        emplace(x);
    }
}

template <typename Key, typename Value>
template <typename Iterator, typename EIterator, typename>
inline unordered_flatmap<Key, Value>::unordered_flatmap(Iterator b, EIterator e) {
    while (b != e) {
        emplace(*b);
        ++b;
    }
}
template <typename Key, typename Value>
template <typename T, typename>
inline auto unordered_flatmap<Key, Value>::operator[](const T& key) -> Value& {
    if (auto i = find(key); i != end()) {
        return i->second;
    }
    this->m_values.emplace_back(key, Value{});
    return this->m_values.back().second;
}

template <typename Key, typename Value>
template <typename T, typename>
inline auto unordered_flatmap<Key, Value>::find(const T& key) noexcept -> iterator {
    auto i = std::find_if(this->m_values.begin(), this->m_values.end(), [&](const auto& elem) { return key == elem.first; });
    return iterator{i};
}

template <typename Key, typename Value>
template <typename T, typename>
inline auto unordered_flatmap<Key, Value>::find(const T& key) const noexcept -> const_iterator {
    return std::find_if(begin(), end(), [&](const auto& elem) { return elem.first == key; });
}

template <typename Key, typename Value> inline auto unordered_flatmap<Key, Value>::insert(const value_type& v) -> std::pair<iterator, bool> {
    if (auto i = find(v.first); i != end()) {
        return {i, false};
    }
    this->m_values.push_back(v);
    return {std::prev(end()), true};
}

template <typename Key, typename Value> inline auto unordered_flatmap<Key, Value>::insert(value_type&& v) -> std::pair<iterator, bool> {
    if (auto i = find(v.first); i != end()) {
        return {i, false};
    }
    this->m_values.push_back(std::move(v));
    return {std::prev(end()), true};
}

template <typename Key, typename Value>
template <typename K, typename V, typename>
inline auto unordered_flatmap<Key, Value>::insert_or_assign(K&& k, V&& v) -> std::pair<iterator, bool> {
    if (auto i = find(k); i != end()) {
        i->second = std::forward<V>(v);
        return {i, false};
    }
    this->m_values.emplace_back(std::forward<K>(k), std::forward<V>(v));
    return {std::prev(end()), true};
}
template <typename Key, typename Value>
template <typename... T, typename>
inline auto unordered_flatmap<Key, Value>::emplace(T&&... t) -> std::pair<iterator, bool> {
    return insert(value_type(std::forward<T>(t)...));
}

template <typename Key, typename Value>
template <typename K, typename... V, typename>
inline auto unordered_flatmap<Key, Value>::try_emplace(K&& key, V&&... v) -> std::pair<iterator, bool> {
    if (auto i = find(key); i != end()) {
        return {i, false};
    }
    this->m_values.emplace_back(std::piecewise_construct, std::forward_as_tuple(std::forward<K>(key)), std::forward_as_tuple(std::forward<V>(v)...));
    return {std::prev(end()), true};
}

template <typename Key, typename Value> inline auto unordered_flatmap<Key, Value>::erase(iterator i) -> void {
    if (i != std::prev(end())) {
        auto addr = std::addressof(*i);
        addr->~value_type();
        new (addr) value_type(std::move(this->m_values.back()));
    }
    this->m_values.pop_back();
}

template <typename Key, typename Value> template <typename K, typename> inline auto unordered_flatmap<Key, Value>::erase(const K& key) -> size_type {
    if (auto i = find(key); i != end()) {
        erase(i);
        return 1;
    }
    return 0;
}

#endif // HEAVYEYELID_FLATMAP_HPP