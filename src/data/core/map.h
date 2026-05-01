#pragma once

#include <cstddef>
#include <initializer_list>
#include <unordered_map>
#include <utility>

#include "data/core/abstract_iterator.h"
#include "data/core/abstract_map.h"

namespace amelia {

struct RuntimeError;
template <typename K, typename V> class MapValueIterator;
template <typename K, typename V> class ConstMapValueIterator;
template <typename K, typename V> class MapKeyIterator;
template <typename K, typename V> class ConstMapKeyIterator;
template <typename K, typename V> class MapPairIterator;
template <typename K, typename V> class ConstMapPairIterator;

template <typename K, typename V> class Map : public AbstractMap<K, V> {
public:
  Map() = default;
  Map(std::initializer_list<std::pair<const K, V>> init) : m_map(init) {}

  bool has(const K &key) const override { return m_map.find(key) != m_map.end(); }
  size_t size() const noexcept override { return m_map.size(); }

  void set(const K &key, V value) override { m_map.insert_or_assign(key, std::move(value)); }

  V &get(const K &key) override {
    if (!has(key)) {
      throw RuntimeError("Key not found in map");
    }
    return m_map.at(key);
  }
  const V &get(const K &key) const override {
    if (!has(key)) {
      throw RuntimeError("Key not found in map");
    }
    return m_map.at(key);
  }

  const V *find(const K &key) const {
    auto it = m_map.find(key);
    if (it == m_map.end()) {
      return nullptr;
    }
    return &(it->second);
  }
  V *find(const K &key) {
    auto it = m_map.find(key);
    if (it == m_map.end()) {
      return nullptr;
    }
    return &(it->second);
  }

  void remove(const K &key) override { m_map.erase(key); }
  V remove_and_get(const K &key) {
    auto it = m_map.find(key);
    if (it == m_map.end()) {
      throw RuntimeError("Key not found in map");
    }
    V value = std::move(it->second);
    m_map.erase(it);
    return value;
  }

  void clear() override { m_map.clear(); }

  V &operator[](const K &key) { return get(key); }
  const V &operator[](const K &key) const { return get(key); }

  bool operator==(const Map<K, V> &other) const { return m_map == other.m_map; }
  bool operator!=(const Map<K, V> &other) const { return m_map != other.m_map; }

  MapPairIterator<K, V> begin() { return MapPairIterator(*this); }
  ConstMapPairIterator<K, V> begin() const { return ConstMapPairIterator(*this); }
  MapPairIterator<K, V> end() { return MapPairIterator(*this).end(); }
  ConstMapPairIterator<K, V> end() const { return ConstMapPairIterator(*this).end(); }

  MapPairIterator<K, V> pairs() { return MapPairIterator(*this); }
  ConstMapPairIterator<K, V> pairs() const { return ConstMapPairIterator(*this); }
  MapKeyIterator<K, V> keys() { return MapKeyIterator(MapPairIterator(*this)); }
  ConstMapKeyIterator<K, V> keys() const {
    return ConstMapKeyIterator(ConstMapPairIterator(*this));
  }
  MapValueIterator<K, V> values() { return MapValueIterator(MapPairIterator(*this)); }
  ConstMapValueIterator<K, V> values() const {
    return ConstMapValueIterator(ConstMapPairIterator(*this));
  }

  friend class MapPairIterator<K, V>;
  friend class ConstMapPairIterator<K, V>;

private:
  std::unordered_map<K, V> m_map;
};

template <typename K, typename V> class MapValueIterator : public AbstractIterator<V &> {
public:
  explicit MapValueIterator(MapPairIterator<K, V> it) : m_it(it) {}

  V &operator*() { return m_it->second; }
  V &peek() override { return **this; }
  V *operator->() { return &(m_it->second); }

  MapValueIterator<K, V> &operator++() {
    ++m_it;
    return *this;
  }

  MapValueIterator<K, V> operator++(int) {
    auto &tmp = *this;
    ++(*this);
    return tmp;
  }

  V &next() override {
    V &value = peek();
    ++(*this);
    return value;
  }

  bool operator==(const MapValueIterator &other) const { return m_it == other.m_it; }
  bool operator!=(const MapValueIterator &other) const { return m_it != other.m_it; }

  MapValueIterator<K, V> begin() const { return *this; }
  MapValueIterator<K, V> end() const { return MapValueIterator(m_it.end()); }

  bool at_end() const override { return m_it.at_end(); }

private:
  MapPairIterator<K, V> m_it;
};

template <typename K, typename V> class MapKeyIterator : public AbstractIterator<const K &> {
public:
  explicit MapKeyIterator(MapPairIterator<K, V> it) : m_it(it) {}

  const K &operator*() { return m_it->first; }
  const K &peek() override { return **this; }

  const K *operator->() { return &(m_it->first); }

  MapKeyIterator<K, V> &operator++() {
    ++m_it;
    return *this;
  }

  MapKeyIterator<K, V> operator++(int) {
    auto &tmp = *this;
    ++(*this);
    return tmp;
  }

  const K &next() override {
    const K &key = peek();
    ++(*this);
    return key;
  }

  bool operator==(const MapKeyIterator &other) const { return m_it == other.m_it; }
  bool operator!=(const MapKeyIterator &other) const { return m_it != other.m_it; }

  MapKeyIterator<K, V> begin() const { return *this; }
  MapKeyIterator<K, V> end() const { return MapKeyIterator(m_it.end()); }

  bool at_end() const override { return m_it.at_end(); }

private:
  MapPairIterator<K, V> m_it;
};

template <typename K, typename V>
class MapPairIterator : public AbstractIterator<std::pair<const K, V> &> {
public:
  using value_type = std::pair<const K, V>;

  explicit MapPairIterator(Map<K, V> &map) : m_begin(map.m_map.begin()), m_end(map.m_map.end()) {}

  value_type &operator*() {
    if (at_end()) {
      throw RuntimeError("Attempted to dereference end iterator");
    }
    return *m_begin;
  }

  value_type &peek() override { return **this; }

  value_type *operator->() {
    if (at_end()) {
      throw RuntimeError("Attempted to dereference end iterator");
    }
    return &(*m_begin);
  }

  MapPairIterator<K, V> &operator++() {
    if (at_end()) {
      throw RuntimeError("Attempted to advance past the end of the map");
    }
    ++m_begin;
    return *this;
  }

  MapPairIterator<K, V> operator++(int) {
    if (at_end()) {
      throw RuntimeError("Attempted to advance past the end of the map");
    }
    auto &tmp = *this;
    ++(*this);
    return tmp;
  }

  value_type &next() override {
    value_type &pair = peek();
    ++(*this);
    return pair;
  }

  bool operator==(const MapPairIterator &other) const { return m_begin == other.m_begin; }
  bool operator!=(const MapPairIterator &other) const { return m_begin != other.m_begin; }

  MapPairIterator<K, V> begin() const { return *this; }
  MapPairIterator<K, V> end() const { return MapPairIterator(m_end, m_end); }

  bool at_end() const override { return m_begin == m_end; }

private:
  MapPairIterator(
      typename std::unordered_map<K, V>::iterator begin,
      typename std::unordered_map<K, V>::iterator end
  )
      : m_begin(begin), m_end(end) {}

  typename std::unordered_map<K, V>::iterator m_begin;
  typename std::unordered_map<K, V>::iterator m_end;
};

template <typename K, typename V> class ConstMapValueIterator : public AbstractIterator<const V &> {
public:
  explicit ConstMapValueIterator(ConstMapPairIterator<K, V> it) : m_it(it) {}

  const V &operator*() const { return m_it->second; }
  const V &peek() override { return **this; }
  const V *operator->() const { return &(m_it->second); }

  ConstMapValueIterator<K, V> &operator++() {
    ++m_it;
    return *this;
  }

  ConstMapValueIterator<K, V> operator++(int) {
    auto &tmp = *this;
    ++(*this);
    return tmp;
  }

  const V &next() override {
    const V &value = peek();
    ++(*this);
    return value;
  }

  bool operator==(const ConstMapValueIterator &other) const { return m_it == other.m_it; }
  bool operator!=(const ConstMapValueIterator &other) const { return m_it != other.m_it; }

  ConstMapValueIterator<K, V> begin() const { return *this; }
  ConstMapValueIterator<K, V> end() const { return ConstMapValueIterator(m_it.end()); }

  bool at_end() const override { return m_it.at_end(); }

private:
  ConstMapPairIterator<K, V> m_it;
};

template <typename K, typename V> class ConstMapKeyIterator : public AbstractIterator<const K &> {
public:
  explicit ConstMapKeyIterator(ConstMapPairIterator<K, V> it) : m_it(it) {}

  const K &operator*() const { return m_it->first; }
  const K &peek() override { return **this; }
  const K *operator->() const { return &(m_it->first); }

  ConstMapKeyIterator<K, V> &operator++() {
    ++m_it;
    return *this;
  }

  ConstMapKeyIterator<K, V> operator++(int) {
    auto &tmp = *this;
    ++(*this);
    return tmp;
  }

  const K &next() override {
    const K &key = peek();
    ++(*this);
    return key;
  }

  bool operator==(const ConstMapKeyIterator &other) const { return m_it == other.m_it; }
  bool operator!=(const ConstMapKeyIterator &other) const { return m_it != other.m_it; }

  ConstMapKeyIterator<K, V> begin() const { return *this; }
  ConstMapKeyIterator<K, V> end() const { return ConstMapKeyIterator(m_it.end()); }

  bool at_end() const override { return m_it.at_end(); }

private:
  ConstMapPairIterator<K, V> m_it;
};

template <typename K, typename V>
class ConstMapPairIterator : public AbstractIterator<const std::pair<const K, V> &> {
public:
  using value_type = std::pair<const K, V>;

  explicit ConstMapPairIterator(const Map<K, V> &map)
      : m_begin(map.m_map.begin()), m_end(map.m_map.end()) {}

  const value_type &operator*() const {
    if (at_end()) {
      throw RuntimeError("Attempted to dereference end iterator");
    }
    return *m_begin;
  }

  const value_type &peek() override { return **this; }

  const value_type *operator->() const {
    if (at_end()) {
      throw RuntimeError("Attempted to dereference end iterator");
    }
    return &(*m_begin);
  }

  ConstMapPairIterator<K, V> &operator++() {
    if (at_end()) {
      throw RuntimeError("Attempted to advance past the end of the map");
    }
    ++m_begin;
    return *this;
  }

  ConstMapPairIterator<K, V> operator++(int) {
    if (at_end()) {
      throw RuntimeError("Attempted to advance past the end of the map");
    }
    auto &tmp = *this;
    ++(*this);
    return tmp;
  }

  const value_type &next() override {
    const value_type &pair = peek();
    ++(*this);
    return pair;
  }

  bool operator==(const ConstMapPairIterator &other) const { return m_begin == other.m_begin; }
  bool operator!=(const ConstMapPairIterator &other) const { return m_begin != other.m_begin; }

  ConstMapPairIterator<K, V> begin() const { return *this; }
  ConstMapPairIterator<K, V> end() const { return ConstMapPairIterator(m_end, m_end); }

  bool at_end() const override { return m_begin == m_end; }

private:
  ConstMapPairIterator(
      typename std::unordered_map<K, V>::const_iterator begin,
      typename std::unordered_map<K, V>::const_iterator end
  )
      : m_begin(begin), m_end(end) {}

  typename std::unordered_map<K, V>::const_iterator m_begin;
  typename std::unordered_map<K, V>::const_iterator m_end;
};

} // namespace amelia
