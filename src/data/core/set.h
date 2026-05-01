#pragma once

#include <cstddef>
#include <initializer_list>
#include <unordered_set>
#include <utility>

namespace amelia {

struct RuntimeError;
template <typename V> class SetIterator;

template <typename V> class Set {
public:
  Set() = default;
  Set(std::initializer_list<V> init) : m_set(init) {}

  bool has(const V &key) const { return m_set.find(key) != m_set.end(); }
  size_t size() const noexcept { return m_set.size(); }

  void add(V value) { m_set.insert(std::move(value)); }

  template <typename... Args> void emplace(Args &&...args) {
    m_set.emplace(std::forward<Args>(args)...);
  }

  void remove(const V &key) { m_set.erase(key); }

  void clear() { m_set.clear(); }

  bool operator==(const Set<V> &other) const { return m_set == other.m_set; }
  bool operator!=(const Set<V> &other) const { return m_set != other.m_set; }

  SetIterator<V> begin() const { return SetIterator(*this); }
  SetIterator<V> end() const { return SetIterator(*this).end(); }

  friend class SetIterator<V>;

private:
  std::unordered_set<V> m_set;
};

template <typename V> class SetIterator {
public:
  explicit SetIterator(const Set<V> &set) : m_begin(set.m_set.begin()), m_end(set.m_set.end()) {}

  const V &operator*() {
    if (at_end()) {
      throw RuntimeError("Attempted to dereference end iterator");
    }
    return *m_begin;
  }

  const V *operator->() {
    if (at_end()) {
      throw RuntimeError("Attempted to dereference end iterator");
    }
    return &(*m_begin);
  }

  SetIterator<V> &operator++() {
    if (at_end()) {
      throw RuntimeError("Attempted to advance past the end of the set");
    }
    ++m_begin;
    return *this;
  }

  SetIterator<V> operator++(int) {
    if (at_end()) {
      throw RuntimeError("Attempted to advance past the end of the set");
    }
    auto &tmp = *this;
    ++(*this);
    return tmp;
  }

  bool operator==(const SetIterator &other) const { return m_begin == other.m_begin; }
  bool operator!=(const SetIterator &other) const { return m_begin != other.m_begin; }

  SetIterator<V> begin() const { return *this; }
  SetIterator<V> end() const { return SetIterator(m_end, m_end); }

  bool at_end() const { return m_begin == m_end; }

private:
  SetIterator(
      typename std::unordered_set<V>::const_iterator begin,
      typename std::unordered_set<V>::const_iterator end
  )
      : m_begin(begin), m_end(end) {}

  typename std::unordered_set<V>::const_iterator m_begin;
  typename std::unordered_set<V>::const_iterator m_end;
};

} // namespace amelia
