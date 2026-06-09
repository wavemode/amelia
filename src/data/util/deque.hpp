#pragma once

#include <cstdlib>

#include "data/util/utility.hpp"

#include "data/util/runtime_error.hpp"

namespace amelia {

namespace internal {
inline unsigned long long chunk_index(unsigned long long item_index) {
  // result is 0 from 0-15, 1 from 16-47, 2 from 48-111, etc.
  return (63ULL - __builtin_clzll(item_index + 16ULL)) - 4ULL;
}

inline unsigned long long chunk_offset(
    unsigned long long chunk_number, unsigned long long global_item_index
) {
  return global_item_index - ((16ULL << chunk_number) - 16ULL);
}

struct chunk_manager {
  void **chunk_ptr = nullptr;
  int32_t elem_size;
  unsigned char chunk_count = 0;
  unsigned char chunk_capacity = 0;

  chunk_manager(int32_t elem_size) : elem_size(elem_size) {}
  chunk_manager(const chunk_manager &) = delete;
  chunk_manager &operator=(const chunk_manager &) = delete;
  chunk_manager(chunk_manager &&other) = delete;
  chunk_manager &operator=(chunk_manager &&other) = delete;

  void destroy() {
    for (unsigned long long i = 0; i < chunk_count; ++i) {
      std::free(chunk_ptr[i]);
    }
    std::free(chunk_ptr);
    chunk_ptr = nullptr;
    chunk_count = 0;
    chunk_capacity = 0;
  }

  void *get_chunk(size_t chunk_number) const {
    return chunk_ptr[chunk_number];
  }

  void grow_chunk_capacity() {
    unsigned char new_capacity = chunk_capacity == 0 ? 4 : chunk_capacity * 2;
    void **new_chunks = static_cast<void **>(std::malloc(sizeof(void *) * new_capacity));
    for (size_t i = 0; i < chunk_count; ++i) {
      new_chunks[i] = chunk_ptr[i];
    }
    std::free(chunk_ptr);
    chunk_ptr = new_chunks;
    chunk_capacity = new_capacity;
  }

  void add_chunk() {
    if (chunk_count == chunk_capacity) {
      grow_chunk_capacity();
    }
    size_t chunk_size = static_cast<size_t>(16) << static_cast<size_t>(chunk_count);
    chunk_ptr[chunk_count] = static_cast<void *>(std::malloc(elem_size * chunk_size));
    ++chunk_count;
  }
};
} // namespace internal

template <typename T> class DequeIterator;
template <typename T> class ConstDequeIterator;

/**
 * Similar to a std::deque, but each subsequent chunk is twice as large as the previous one.
 *
 * The initial chunk has a size of 16.
 */
template <typename T> class Deque {
public:
  Deque() = default;

  template <size_t N> explicit Deque(const T (&arr)[N]) {
    for (size_t i = 0; i < N; ++i) {
      push_back(arr[i]);
    }
  }

  Deque(const Deque &other) {
    clone(other);
  }

  Deque(Deque &&other) noexcept {
    move_from(move(other));
  }

  Deque &operator=(const Deque &other) {
    if (this != &other) {
      while (m_size > 0) {
        pop_back();
      }
      clone(other);
    }
    return *this;
  }

  Deque &operator=(Deque &&other) noexcept {
    if (this != &other) {
      while (m_size > 0) {
        pop_back();
      }
      move_from(move(other));
    }
    return *this;
  }

  ~Deque() {
    while (m_size > 0) {
      pop_back();
    }
    m_positive_chunks.destroy();
    m_negative_chunks.destroy();
  }

  void push_back(T value) {
    T *item_ptr = insert_item(m_start + m_size);
    new (item_ptr) T(move(value));
    ++m_size;
  }

  template <typename... Args> T &emplace_back(Args &&...args) {
    T *item_ptr = insert_item(m_start + m_size);
    new (item_ptr) T(amelia::forward<Args>(args)...);
    ++m_size;
    return *item_ptr;
  }

  void pop_back() {
    if (m_size == 0) {
      throw RuntimeError("Cannot pop_back from empty Deque");
    }
    get_item(m_start + m_size - 1)->~T();
    --m_size;
  }

  void push_front(T value) {
    T *item_ptr = insert_item(m_start - 1);
    new (item_ptr) T(move(value));
    --m_start;
    ++m_size;
  }

  template <typename... Args> T &emplace_front(Args &&...args) {
    T *item_ptr = insert_item(m_start - 1);
    new (item_ptr) T(amelia::forward<Args>(args)...);
    --m_start;
    ++m_size;
    return *item_ptr;
  }

  void pop_front() {
    if (m_size == 0) {
      throw RuntimeError("Cannot pop_front from empty Deque");
    }
    get_item(m_start)->~T();
    ++m_start;
    --m_size;
  }

  T &get(size_t index) {
    if (static_cast<int64_t>(index) >= m_size) {
      throw RuntimeError("Index out of bounds in Deque::get");
    }
    return *get_item(m_start + static_cast<int64_t>(index));
  }

  T &get(size_t index) const {
    if (static_cast<int64_t>(index) >= m_size) {
      throw RuntimeError("Index out of bounds in Deque::get");
    }
    return *get_item(m_start + static_cast<int64_t>(index));
  }

  size_t size() const {
    return m_size;
  }

  DequeIterator<T> begin() {
    return DequeIterator<T>(*this, 0);
  }

  DequeIterator<T> end() {
    return DequeIterator<T>(*this, m_size);
  }

  ConstDequeIterator<T> begin() const {
    return ConstDequeIterator<T>(*this, 0);
  }

  ConstDequeIterator<T> end() const {
    return ConstDequeIterator<T>(*this, m_size);
  }

  T &operator[](size_t index) {
    return get(index);
  }

  const T &operator[](size_t index) const {
    return get(index);
  }

  bool operator==(const Deque &other) const {
    if (m_size != other.m_size) {
      return false;
    }
    for (size_t i = 0; i < m_size; ++i) {
      if (get(i) != other.get(i)) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(const Deque &other) const {
    return !(*this == other);
  }

private:
  T *get_item(int64_t global_index) const {
    if (global_index >= 0) {
      unsigned long long chunk_number = internal::chunk_index(global_index);
      size_t offset = internal::chunk_offset(chunk_number, global_index);
      return static_cast<T *>(m_positive_chunks.get_chunk(chunk_number)) + offset;
    } else {
      unsigned long long chunk_number = internal::chunk_index(-1LL - global_index);
      size_t offset = internal::chunk_offset(chunk_number, -1LL - global_index);
      return static_cast<T *>(m_negative_chunks.get_chunk(chunk_number)) + offset;
    }
  }

  T *insert_item(int64_t global_index) {
    if (global_index >= 0) {
      unsigned long long chunk_number = internal::chunk_index(global_index);
      while (chunk_number >= m_positive_chunks.chunk_count) {
        m_positive_chunks.add_chunk();
      }
      size_t offset = internal::chunk_offset(chunk_number, global_index);
      T *item_ptr = static_cast<T *>(m_positive_chunks.get_chunk(chunk_number)) + offset;
      return item_ptr;
    } else {
      unsigned long long chunk_number = internal::chunk_index(-1LL - global_index);
      while (chunk_number >= m_negative_chunks.chunk_count) {
        m_negative_chunks.add_chunk();
      }
      size_t offset = internal::chunk_offset(chunk_number, -1LL - global_index);
      T *item_ptr = static_cast<T *>(m_negative_chunks.get_chunk(chunk_number)) + offset;
      return item_ptr;
    }
  }

  void clone(const Deque &other) {
    for (size_t i = 0; i < other.m_size; ++i) {
      push_back(other.get(i));
    }
  }

  void move_from(Deque &&other) {
    m_positive_chunks.chunk_ptr = other.m_positive_chunks.chunk_ptr;
    m_positive_chunks.chunk_count = other.m_positive_chunks.chunk_count;
    m_positive_chunks.chunk_capacity = other.m_positive_chunks.chunk_capacity;
    other.m_positive_chunks.chunk_ptr = nullptr;
    other.m_positive_chunks.chunk_count = 0;
    other.m_positive_chunks.chunk_capacity = 0;

    m_negative_chunks.chunk_ptr = other.m_negative_chunks.chunk_ptr;
    m_negative_chunks.chunk_count = other.m_negative_chunks.chunk_count;
    m_negative_chunks.chunk_capacity = other.m_negative_chunks.chunk_capacity;
    other.m_negative_chunks.chunk_ptr = nullptr;
    other.m_negative_chunks.chunk_count = 0;
    other.m_negative_chunks.chunk_capacity = 0;

    m_start = other.m_start;
    m_size = other.m_size;
    other.m_start = 0;
    other.m_size = 0;
  }

  int64_t m_start = 0;
  int64_t m_size = 0;
  internal::chunk_manager m_positive_chunks{sizeof(T)};
  internal::chunk_manager m_negative_chunks{sizeof(T)};
};

template <typename T> class DequeIterator {
public:
  DequeIterator(Deque<T> &deque, size_t index) : m_deque(&deque), m_index(index) {}

  T &peek() {
    return m_deque->get(m_index);
  }

  T &next() {
    if (at_end()) {
      throw RuntimeError("Attempted to advance DequeIterator past the end");
    }
    T &item = peek();
    ++m_index;
    return item;
  }

  bool at_end() const {
    return m_index >= m_deque->size();
  }

  bool operator==(const DequeIterator &other) const {
    return m_deque == other.m_deque && m_index == other.m_index;
  }

  bool operator!=(const DequeIterator &other) const {
    return !(*this == other);
  }

  DequeIterator &operator++() {
    next();
    return *this;
  }

  DequeIterator operator++(int) {
    DequeIterator temp = *this;
    next();
    return temp;
  }

  T &operator*() {
    return peek();
  }

  T *operator->() {
    return &peek();
  }

private:
  Deque<T> *m_deque;
  size_t m_index;
};

template <typename T> class ConstDequeIterator {
public:
  ConstDequeIterator(const Deque<T> &deque, size_t index) : m_deque(&deque), m_index(index) {}

  const T &peek() const {
    return m_deque->get(m_index);
  }

  const T &next() {
    if (at_end()) {
      throw RuntimeError("Attempted to advance ConstDequeIterator past the end");
    }
    const T &item = peek();
    ++m_index;
    return item;
  }

  bool at_end() const {
    return m_index >= m_deque->size();
  }

  bool operator==(const ConstDequeIterator &other) const {
    return m_deque == other.m_deque && m_index == other.m_index;
  }

  bool operator!=(const ConstDequeIterator &other) const {
    return !(*this == other);
  }

  ConstDequeIterator &operator++() {
    next();
    return *this;
  }

  ConstDequeIterator operator++(int) {
    ConstDequeIterator temp = *this;
    next();
    return temp;
  }

  const T &operator*() const {
    return peek();
  }

  const T *operator->() const {
    return &peek();
  }

private:
  const Deque<T> *m_deque;
  size_t m_index;
};

} // namespace amelia
