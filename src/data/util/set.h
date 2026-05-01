#pragma once

#include <cstddef>
#include <initializer_list>
#include <unordered_set>
#include <utility>

#include "data/util/abstract_iterator.h"
#include "data/util/abstract_set.h"

namespace amelia {

struct RuntimeError;
template <typename T> class SetIterator;

template <typename T> class Set : public AbstractSet<T> {
public:
  Set() = default;
  Set(std::initializer_list<T> init) : m_set(init) {}

  bool has(const T &key) const override { return m_set.find(key) != m_set.end(); }
  size_t size() const noexcept override { return m_set.size(); }

  void add(T value) override { m_set.insert(std::move(value)); }

  template <typename... Args> void emplace(Args &&...args) {
    m_set.emplace(std::forward<Args>(args)...);
  }

  void remove(const T &key) override { m_set.erase(key); }

  void clear() override { m_set.clear(); }

  bool operator==(const Set<T> &other) const { return m_set == other.m_set; }
  bool operator!=(const Set<T> &other) const { return m_set != other.m_set; }

  SetIterator<T> begin() const { return SetIterator(*this); }
  SetIterator<T> end() const { return SetIterator(*this).end(); }

  friend class SetIterator<T>;

private:
  std::unordered_set<T> m_set;
};

template <typename T> class SetIterator : public AbstractIterator<const T &> {
public:
  explicit SetIterator(const Set<T> &set) : m_begin(set.m_set.begin()), m_end(set.m_set.end()) {}

  const T &operator*() {
    if (at_end()) {
      throw RuntimeError("Attempted to dereference end iterator");
    }
    return *m_begin;
  }

  const T &peek() override { return **this; }

  const T *operator->() {
    if (at_end()) {
      throw RuntimeError("Attempted to dereference end iterator");
    }
    return &(*m_begin);
  }

  SetIterator<T> &operator++() {
    if (at_end()) {
      throw RuntimeError("Attempted to advance past the end of the set");
    }
    ++m_begin;
    return *this;
  }

  SetIterator<T> operator++(int) {
    if (at_end()) {
      throw RuntimeError("Attempted to advance past the end of the set");
    }
    auto &tmp = *this;
    ++(*this);
    return tmp;
  }

  const T &next() override {
    const T &value = peek();
    ++(*this);
    return value;
  }

  bool operator==(const SetIterator &other) const { return m_begin == other.m_begin; }
  bool operator!=(const SetIterator &other) const { return m_begin != other.m_begin; }

  SetIterator<T> begin() const { return *this; }
  SetIterator<T> end() const { return SetIterator(m_end, m_end); }

  bool at_end() const override { return m_begin == m_end; }

private:
  SetIterator(
      typename std::unordered_set<T>::const_iterator begin,
      typename std::unordered_set<T>::const_iterator end
  )
      : m_begin(begin), m_end(end) {}

  typename std::unordered_set<T>::const_iterator m_begin;
  typename std::unordered_set<T>::const_iterator m_end;
};

} // namespace amelia
