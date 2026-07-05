#pragma once

#include <cstddef>

#include "util/data/abstract_set.hpp"
#include "util/data/list.hpp"
#include "util/data/slice.hpp"

namespace amelia {

namespace internal {

template <typename T> struct hash_set_table_element {
  uint64_t hash;
  T value;
};

template <typename T> struct hash_set_table {
  hash_set_table_element<T> *elems = nullptr;
  size_t table_size = 0;
  size_t remaining_elems = 0;
  size_t live_elems = 0;

  hash_set_table() = default;

  hash_set_table(const hash_set_table &other) {
    for (size_t i = 0; i < other.table_size; ++i) {
      const hash_set_table_element<T> &other_elem = other.elems[i];
      if (is_first_bit_set_to_1(other_elem.hash)) {
        put(T(other_elem.value));
      }
    }
  }

  hash_set_table(hash_set_table &&other) noexcept
      : elems(other.elems), table_size(other.table_size), remaining_elems(other.remaining_elems),
        live_elems(other.live_elems) {
    other.elems = nullptr;
    other.table_size = 0;
    other.remaining_elems = 0;
    other.live_elems = 0;
  }

  ~hash_set_table() {
    clear();
    std::free(elems);
  }

  hash_set_table &operator=(const hash_set_table &other) {
    if (this != &other) {
      clear();
      for (size_t i = 0; i < other.table_size; ++i) {
        const hash_set_table_element<T> &other_elem = other.elems[i];
        if (is_first_bit_set_to_1(other_elem.hash)) {
          put(T(other_elem.value));
        }
      }
    }
    return *this;
  }

  hash_set_table &operator=(hash_set_table &&other) noexcept {
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
      hash_set_table_element<T> &table_elem = elems[i];
      if (is_first_bit_set_to_1(table_elem.hash)) {
        table_elem.value.~T();
        table_elem.hash = set_2_highest_bits_to_01(table_elem.hash);
        --live_elems;
      }
    }
  }

  void grow() {
    hash_set_table<T> new_table;
    new_table.table_size = table_size == 0 ? 16 : table_size * 2;
    new_table.elems = static_cast<hash_set_table_element<T> *>(
        std::calloc(new_table.table_size, sizeof(hash_set_table_element<T>))
    );
    if (!new_table.elems) {
      std::abort();
    }
    new_table.remaining_elems = (new_table.table_size * 3) / 4;

    for (size_t i = 0; i < table_size; ++i) {
      hash_set_table_element<T> &table_elem = elems[i];
      if (is_first_bit_set_to_1(table_elem.hash)) {
        // this slot is in use, so we need to insert this element into the new table
        new_table.put(move(table_elem.value));
      }
    }

    *this = move(new_table);
  }

  void put(T &&value) {
    if (remaining_elems == 0) {
      grow();
    }
    uint64_t hash = set_2_highest_bits_to_00(amelia::hash(value));
    size_t index = hash % table_size;
    size_t probe_length = 0;
    while (true) {
      hash_set_table_element<T> &table_elem = elems[index];
      if (!is_first_bit_set_to_1(table_elem.hash)) {
        // this slot is empty, so we can insert a new element here
        break;
      }

      // this table element is in use

      // check if the value of the table element matches the value we're inserting
      uint64_t table_elem_hash = set_2_highest_bits_to_00(table_elem.hash);
      if (table_elem_hash == hash && table_elem.value == value) {
        // the values match, so we can simply return
        return;
      }

      // the values don't match

      // first, let's check the distance of the table element from its ideal position
      size_t table_elem_ideal_position = table_elem_hash % table_size;
      size_t table_elem_distance_from_ideal = (table_size - (table_elem_ideal_position - index)) %
                                              table_size;

      // if we have probed farther from our ideal position than the table element did from its ideal
      // position, let's just store the new element here, and find a new place for the table element
      if (probe_length > table_elem_distance_from_ideal) {
        table_elem.hash = get_two_highest_bits(table_elem.hash) | hash;
        hash = table_elem_hash;
        swap(value, table_elem.value);
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
    new (&elems[index].value) T(move(value));
    ++live_elems;
  }

  int64_t index_of(const T &value) const {
    if (table_size == 0) {
      return -1;
    }
    uint64_t hash = set_2_highest_bits_to_00(amelia::hash(value));
    size_t index = hash % table_size;
    size_t probe_length = 0;
    while (true) {
      hash_set_table_element<T> &table_elem = elems[index];
      uint64_t table_elem_hash = set_2_highest_bits_to_00(table_elem.hash);

      if (is_first_bit_set_to_1(table_elem.hash)) {
        if (table_elem_hash == hash && table_elem.value == value) {
          return index;
        }

        if (!is_second_bit_set_to_1(table_elem.hash)) {
          // This is not a reused tombstone, so (due to the fairness created in put() via swapping)
          // this element's distance-from-ideal must be less than or equal to the
          // distance-from-ideal of every other element in the table. If we've currently searched
          // farther than that, then our value is not in the table.
          size_t table_elem_ideal_position = table_elem_hash % table_size;
          size_t table_elem_distance_from_ideal = (table_size - (table_elem_ideal_position - index)
                                                  ) %
                                                  table_size;
          if (probe_length > table_elem_distance_from_ideal) {
            return -1;
          }
        }
      } else if (!is_second_bit_set_to_1(table_elem.hash)) {
        // we've reached an empty, never-used slot, so the value isn't in the table
        return -1;
      }
      index = (index + 1) % table_size;
      ++probe_length;
    }
  }

  void remove(const T &value) {
    int64_t index = index_of(value);
    if (index == -1) {
      return;
    }
    hash_set_table_element<T> &table_elem = elems[index];
    --live_elems;
    table_elem.value.~T();
    // leave behind a tombstone (first bit means unused, second bit means it was previously used)
    table_elem.hash = set_2_highest_bits_to_01(table_elem.hash);
  }

  bool has(const T &value) const {
    return index_of(value) != -1;
  }
};

} // namespace internal

template <typename T> class SetIterator;

template <typename T> class Set : public AbstractSet<T> {
public:
  Set() = default;

  template <size_t N> Set(const T (&values)[N]) {
    for (size_t i = 0; i < N; ++i) {
      add(values[i]);
    }
  }

  bool has(const T &key) const override {
    return m_table.has(key);
  }
  size_t size() const noexcept override {
    return m_table.live_elems;
  }

  void add(T value) override {
    m_table.put(move(value));
  }

  void add_all(Slice<T> values) {
    for (const T &value : values) {
      m_table.put(T(value));
    }
  }

  void add_all(List<T> &&values) {
    for (T &value : values) {
      m_table.put(move(value));
    }
    values.clear();
  }

  void remove(const T &key) override {
    m_table.remove(key);
  }

  void clear() override {
    m_table.clear();
  }

  SetIterator<T> begin() const {
    return SetIterator<T>(*this);
  }

  SetIterator<T> end() const {
    return SetIterator<T>(*this).end();
  }

  bool operator==(const Set<T> &other) const {
    if (size() != other.size()) {
      return false;
    }
    for (const auto &value : *this) {
      if (!other.has(value)) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(const Set<T> &other) const {
    return !(*this == other);
  }

  friend class SetIterator<T>;

private:
  internal::hash_set_table<T> m_table;
};

template <typename T> class SetIterator {
public:
  SetIterator(const Set<T> &set)
      : SetIterator(set.m_table.elems, set.m_table.elems + set.m_table.table_size) {}

  const T &peek() {
    if (at_end()) {
      throw RuntimeError("Attempted to peek past end of iterator");
    }
    return it()->value;
  }

  const T &next() {
    if (at_end()) {
      throw RuntimeError("Attempted to next past end of iterator");
    }
    const T &value = peek();
    do {
      m_it = it() + 1;
    } while (m_it != m_end && !internal::is_first_bit_set_to_1(it()->hash));
    return value;
  }

  bool at_end() const noexcept {
    return m_it == m_end;
  }

  SetIterator<T> begin() {
    return *this;
  }

  SetIterator<T> begin() const {
    return *this;
  }

  SetIterator<T> end() {
    return SetIterator<T>(m_end, m_end);
  }

  SetIterator<T> end() const {
    return SetIterator<T>(m_end, m_end);
  }

  const T &operator*() {
    return peek();
  }

  SetIterator<T> &operator++() {
    next();
    return *this;
  }

  SetIterator<T> operator++(int) {
    auto &tmp = *this;
    next();
    return tmp;
  }

  bool operator==(const SetIterator &other) const {
    return m_it == other.m_it;
  }

  bool operator!=(const SetIterator &other) const {
    return m_it != other.m_it;
  }

private:
  SetIterator(void *begin, void *end) : m_it(begin), m_end(end) {
    while (m_it != m_end && !internal::is_first_bit_set_to_1(it()->hash)) {
      m_it = it() + 1;
    }
  }

  void *m_it;
  void *m_end;

  internal::hash_set_table_element<T> *it() const {
    return static_cast<internal::hash_set_table_element<T> *>(m_it);
  }
};

} // namespace amelia
