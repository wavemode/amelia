#pragma once

#include <cstddef>
#include <cstdlib>

#include "util/data/abstract_map.hpp"
#include "util/data/option.hpp"
#include "util/data/pair.hpp"
#include "util/data/ref.hpp"
#include "util/data/runtime_error.hpp"
#include "util/data/utility.hpp"

namespace amelia {

namespace internal {

template <typename K, typename V> struct hash_map_table_element {
  uint64_t hash;
  Pair<K, V> pair;
};

template <typename K, typename V> struct hash_map_table {
  hash_map_table_element<K, V> *elems = nullptr;
  size_t table_size = 0;
  size_t remaining_elems = 0;
  size_t live_elems = 0;

  hash_map_table() = default;

  hash_map_table(const hash_map_table &other) {
    for (size_t i = 0; i < other.table_size; ++i) {
      const hash_map_table_element<K, V> &other_elem = other.elems[i];
      if (is_first_bit_set_to_1(other_elem.hash)) {
        put(K(other_elem.pair.first), V(other_elem.pair.second));
      }
    }
  }

  hash_map_table(hash_map_table &&other) noexcept
      : elems(other.elems), table_size(other.table_size), remaining_elems(other.remaining_elems),
        live_elems(other.live_elems) {
    other.elems = nullptr;
    other.table_size = 0;
    other.remaining_elems = 0;
    other.live_elems = 0;
  }

  ~hash_map_table() {
    clear();
    std::free(elems);
  }

  hash_map_table &operator=(const hash_map_table &other) {
    if (this != &other) {
      clear();
      for (size_t i = 0; i < other.table_size; ++i) {
        const hash_map_table_element<K, V> &other_elem = other.elems[i];
        if (is_first_bit_set_to_1(other_elem.hash)) {
          put(K(other_elem.pair.first), V(other_elem.pair.second));
        }
      }
    }
    return *this;
  }

  hash_map_table &operator=(hash_map_table &&other) noexcept {
    if (this != &other) {
      clear();
      std::free(elems);
      elems = other.elems;
      table_size = other.table_size;
      remaining_elems = other.remaining_elems;
      live_elems = other.live_elems;
      other.elems = nullptr;
      other.table_size = 0;
      other.remaining_elems = 0;
      other.live_elems = 0;
    }
    return *this;
  }

  void clear() noexcept {
    for (size_t i = 0; i < table_size; ++i) {
      hash_map_table_element<K, V> &table_elem = elems[i];
      if (is_first_bit_set_to_1(table_elem.hash)) {
        table_elem.pair.first.~K();
        table_elem.pair.second.~V();
        table_elem.hash = set_2_highest_bits_to_01(table_elem.hash);
        --live_elems;
      }
    }
  }

  void grow() {
    hash_map_table<K, V> new_table;
    new_table.table_size = table_size == 0 ? 16 : table_size * 2;
    new_table.elems = static_cast<hash_map_table_element<K, V> *>(
        std::calloc(new_table.table_size, sizeof(hash_map_table_element<K, V>))
    );
    if (!new_table.elems) {
      std::abort();
    }
    new_table.remaining_elems = (new_table.table_size * 3) / 4;

    for (size_t i = 0; i < table_size; ++i) {
      hash_map_table_element<K, V> &table_elem = elems[i];
      if (is_first_bit_set_to_1(table_elem.hash)) {
        // this slot is in use, so we need to insert this element into the new table
        new_table.put_with_hash(
            set_2_highest_bits_to_00(table_elem.hash),
            move(table_elem.pair.first),
            move(table_elem.pair.second)
        );
      }
    }

    *this = move(new_table);
  }

  void put(K &&key, V &&value) {
    uint64_t hash = set_2_highest_bits_to_00(amelia::hash(key));
    put_with_hash(hash, move(key), move(value));
  }

  void put_with_hash(uint64_t hash, K &&key, V &&value) {
    if (remaining_elems == 0) {
      grow();
    }
    size_t index = hash % table_size;
    size_t probe_length = 0;
    while (true) {
      hash_map_table_element<K, V> &table_elem = elems[index];
      if (!is_first_bit_set_to_1(table_elem.hash)) {
        // this slot is empty, so we can insert a new element here
        break;
      }

      // this table element is in use

      // check if the key of the table element matches the key we're inserting
      uint64_t table_elem_hash = set_2_highest_bits_to_00(table_elem.hash);
      if (table_elem_hash == hash && table_elem.pair.first == key) {
        // the keys match, so we can simply update the value and return
        table_elem.pair.second = move(value);
        return;
      }

      // the keys don't match

      // first, let's check the distance of the table element from its ideal position
      size_t table_elem_ideal_position = table_elem_hash % table_size;
      size_t table_elem_distance_from_ideal = (table_size - (table_elem_ideal_position - index)) %
                                              table_size;

      // if we have probed farther from our ideal position than the table element did from its ideal
      // position, let's just store the new element here, and find a new place for the table element
      if (probe_length > table_elem_distance_from_ideal) {
        table_elem.hash = get_two_highest_bits(table_elem.hash) | hash;
        hash = table_elem_hash;
        swap(key, table_elem.pair.first);
        swap(value, table_elem.pair.second);
        probe_length = table_elem_distance_from_ideal;
      }

      index = (index + 1) % table_size;
      ++probe_length;
    }

    // we found an empty slot, so we can insert the new element here
    if (is_second_bit_set_to_1(elems[index].hash)) {
      // we are reusing a tombstone, so the second bit needs to be 1 and we need to not decrement
      // the remaining_elems count (since we are not actually using up a new slot in the table)
      elems[index].hash = set_2_highest_bits_to_11(hash);
    } else {
      elems[index].hash = set_2_highest_bits_to_10(hash);
      --remaining_elems;
    }
    new (&elems[index].pair.first) K(move(key));
    new (&elems[index].pair.second) V(move(value));
    ++live_elems;
  }

  int64_t index_of(const K &key) const {
    if (table_size == 0) {
      return -1;
    }
    uint64_t hash = set_2_highest_bits_to_00(amelia::hash(key));
    size_t index = hash % table_size;
    size_t probe_length = 0;
    while (true) {
      hash_map_table_element<K, V> &table_elem = elems[index];
      uint64_t table_elem_hash = set_2_highest_bits_to_00(table_elem.hash);

      if (is_first_bit_set_to_1(table_elem.hash)) {
        if (table_elem_hash == hash && table_elem.pair.first == key) {
          return index;
        }

        if (!is_second_bit_set_to_1(table_elem.hash)) {
          // This is not a reused tombstone, so (due to the fairness created in put() via swapping)
          // this element's distance-from-ideal must be less than or equal to the
          // distance-from-ideal of every other element in the table. If we've currently searched
          // farther than that, then our key is not in the table.
          size_t table_elem_ideal_position = table_elem_hash % table_size;
          size_t table_elem_distance_from_ideal = (table_size - (table_elem_ideal_position - index)
                                                  ) %
                                                  table_size;
          if (probe_length > table_elem_distance_from_ideal) {
            return -1;
          }
        }
      } else if (!is_second_bit_set_to_1(table_elem.hash)) {
        // we've reached an empty, never-used slot, so the key isn't in the table
        return -1;
      }
      index = (index + 1) % table_size;
      ++probe_length;
    }
  }

  void remove(const K &key) {
    int64_t index = index_of(key);
    if (index == -1) {
      return;
    }
    hash_map_table_element<K, V> &table_elem = elems[index];
    --live_elems;
    table_elem.pair.first.~K();
    table_elem.pair.second.~V();
    // leave behind a tombstone (first bit means unused, second bit means it was previously used)
    table_elem.hash = set_2_highest_bits_to_01(table_elem.hash);
  }

  bool has(const K &key) const {
    return index_of(key) != -1;
  }
};

} // namespace internal

template <typename K, typename V> class MapKeyIterator;
template <typename K, typename V> class MapValueIterator;
template <typename K, typename V> class MapPairIterator;

template <typename K, typename V> struct MapPair {
  const K &first;
  V &second;
};

template <typename K, typename V> class Map : public AbstractMap<K, V> {
public:
  Map() = default;

  template <size_t N> explicit Map(const Pair<K, V> (&array)[N]) {
    for (size_t i = 0; i < N; ++i) {
      set(array[i].first, array[i].second);
    }
  }

  bool has(const K &key) const override {
    return m_table.has(key);
  }
  size_t size() const noexcept override {
    return m_table.live_elems;
  }

  void set(const K &key, V value) override {
    m_table.put(K(key), move(value));
  }

  V &get(const K &key) override {
    int64_t index = m_table.index_of(key);
    if (index == -1) {
      throw RuntimeError("Attempted to get a key that doesn't exist in the Map");
    }
    return m_table.elems[index].pair.second;
  }

  const V &get(const K &key) const override {
    int64_t index = m_table.index_of(key);
    if (index == -1) {
      throw RuntimeError("Attempted to get a key that doesn't exist in the Map");
    }
    return m_table.elems[index].pair.second;
  }

  Option<Ref<V>> find(const K &key) {
    int64_t index = m_table.index_of(key);
    if (index == -1) {
      return None();
    }
    return Some(Ref(m_table.elems[index].pair.second));
  }

  Option<Ref<const V>> find(const K &key) const {
    int64_t index = m_table.index_of(key);
    if (index == -1) {
      return None();
    }
    return Some(Ref(static_cast<const V &>(m_table.elems[index].pair.second)));
  }

  void remove(const K &key) override {
    m_table.remove(key);
  }

  V remove_and_get(const K &key) {
    int64_t index = m_table.index_of(key);
    if (index == -1) {
      throw RuntimeError("Attempted to remove_and_get a key that doesn't exist in the Map");
    }
    V value = move(m_table.elems[index].pair.second);
    m_table.remove(key);
    return value;
  }

  void clear() override {
    m_table.clear();
  }

  MapPairIterator<K, V> begin() {
    return MapPairIterator<K, V>(*this);
  }

  MapPairIterator<K, const V> begin() const {
    return MapPairIterator<K, const V>(*this);
  }

  MapPairIterator<K, V> end() {
    return MapPairIterator<K, V>(*this).end();
  }

  MapPairIterator<K, const V> end() const {
    return MapPairIterator<K, const V>(*this).end();
  }

  MapPairIterator<K, V> pairs() {
    return begin();
  }

  MapPairIterator<K, const V> pairs() const {
    return begin();
  }

  MapKeyIterator<K, V> keys() {
    return MapKeyIterator(begin());
  }

  MapKeyIterator<K, const V> keys() const {
    return MapKeyIterator(begin());
  }

  MapValueIterator<K, V> values() {
    return MapValueIterator(begin());
  }

  MapValueIterator<K, const V> values() const {
    return MapValueIterator(begin());
  }

  V &operator[](const K &key) {
    return get(key);
  }

  const V &operator[](const K &key) const {
    return get(key);
  }

  bool operator==(const Map<K, V> &other) const {
    if (size() != other.size()) {
      return false;
    }
    for (const auto &pair : *this) {
      const K &key = pair.first;
      const V &value = pair.second;
      size_t other_index = other.m_table.index_of(key);
      if (other_index == -1 || other.m_table.elems[other_index].pair.second != value) {
        return false;
      }
    }
    return true;
  }
  bool operator!=(const Map<K, V> &other) const {
    return !(*this == other);
  }

  friend class MapPairIterator<K, V>;
  friend class MapPairIterator<K, const V>;
  friend class MapPairIterator<const K, V>;
  friend class MapPairIterator<const K, const V>;

private:
  internal::hash_map_table<K, V> m_table;
};

template <typename K, typename V> class MapPairIterator {
public:
  template <typename M>
  explicit MapPairIterator(const M &map)
      : MapPairIterator(map.m_table.elems, map.m_table.elems + map.m_table.table_size) {}

  bool at_end() const noexcept {
    return m_it == m_end;
  }

  MapPair<K, V> peek() const {
    if (at_end()) {
      throw RuntimeError("Attempted to peek past end of iterator");
    }
    return MapPair<K, V>{it()->pair.first, it()->pair.second};
  }

  MapPair<K, V> next() {
    if (at_end()) {
      throw RuntimeError("Attempted to next past end of iterator");
    }
    MapPair<K, V> pair = peek();
    do {
      m_it = it() + 1;
    } while (m_it != m_end && !internal::is_first_bit_set_to_1(it()->hash));
    return pair;
  }

  MapPairIterator<K, V> begin() {
    return *this;
  }

  MapPairIterator<K, const V> begin() const {
    return *this;
  }

  MapPairIterator<K, V> end() {
    return MapPairIterator<K, V>(m_end, m_end);
  }

  MapPairIterator<K, const V> end() const {
    return MapPairIterator<K, V>(m_end, m_end);
  }

  MapPairIterator<K, V> &operator++() {
    next();
    return *this;
  }

  MapPairIterator<K, V> operator++(int) {
    auto &tmp = *this;
    next();
    return tmp;
  }

  MapPair<K, V> operator*() {
    return peek();
  }

  bool operator==(const MapPairIterator &other) const {
    return m_it == other.m_it;
  }

  bool operator!=(const MapPairIterator &other) const {
    return m_it != other.m_it;
  }

private:
  MapPairIterator(void *begin, void *end) : m_it(begin), m_end(end) {
    while (m_it != m_end && !internal::is_first_bit_set_to_1(it()->hash)) {
      m_it = it() + 1;
    }
  }

  void *m_it;
  void *m_end;

  internal::hash_map_table_element<K, V> *it() const {
    return static_cast<internal::hash_map_table_element<K, V> *>(m_it);
  }
};

template <typename K, typename V> class MapValueIterator {
public:
  MapValueIterator(MapPairIterator<K, V> pair_it) : m_pair_it(pair_it) {}

  bool at_end() const noexcept {
    return m_pair_it.at_end();
  }

  V &peek() const {
    return m_pair_it.peek().second;
  }

  V &next() {
    return m_pair_it.next().second;
  }

  MapValueIterator<K, V> begin() {
    return *this;
  }

  MapValueIterator<K, const V> begin() const {
    return *this;
  }

  MapValueIterator<K, V> end() {
    return MapValueIterator(m_pair_it.end());
  }

  MapValueIterator<K, const V> end() const {
    return MapValueIterator(m_pair_it.end());
  }

  MapValueIterator<K, V> &operator++() {
    next();
    return *this;
  }

  MapValueIterator<K, V> operator++(int) {
    auto &tmp = *this;
    next();
    return tmp;
  }

  V &operator*() {
    return peek();
  }

  bool operator==(const MapValueIterator &other) const {
    return m_pair_it == other.m_pair_it;
  }

  bool operator!=(const MapValueIterator &other) const {
    return m_pair_it != other.m_pair_it;
  }

private:
  MapPairIterator<K, V> m_pair_it;
};

template <typename K, typename V> class MapKeyIterator {
public:
  MapKeyIterator(MapPairIterator<K, V> pair_it) : m_pair_it(pair_it) {}

  bool at_end() const noexcept {
    return m_pair_it.at_end();
  }

  const K &peek() const {
    return m_pair_it.peek().first;
  }

  const K &next() {
    return m_pair_it.next().first;
  }

  MapKeyIterator<K, V> begin() {
    return *this;
  }

  MapKeyIterator<K, const V> begin() const {
    return *this;
  }

  MapKeyIterator<K, V> end() {
    return MapKeyIterator(m_pair_it.end());
  }

  MapKeyIterator<K, const V> end() const {
    return MapKeyIterator(m_pair_it.end());
  }

  MapKeyIterator<K, V> &operator++() {
    next();
    return *this;
  }

  MapKeyIterator<K, V> operator++(int) {
    auto &tmp = *this;
    next();
    return tmp;
  }

  const K &operator*() const {
    return peek();
  }

  bool operator==(const MapKeyIterator &other) const {
    return m_pair_it == other.m_pair_it;
  }

  bool operator!=(const MapKeyIterator &other) const {
    return m_pair_it != other.m_pair_it;
  }

private:
  MapPairIterator<K, V> m_pair_it;
};

} // namespace amelia
