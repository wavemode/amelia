#pragma once

#include <climits>

#include "util/data/runtime_error.hpp"
#include "util/data/utility.hpp"

namespace amelia {

namespace internal {
struct ControlBlock {
  uint32_t ref_counts[3] = {0, 0, UINT32_MAX}; // strong_count, weak_count, sentinel

  static uint32_t *make() {
    ControlBlock *block = new ControlBlock();
    return block->ref_counts;
  }

  static bool is_strong(uint32_t *ref_count_ptr) {
    return *(ref_count_ptr + 1) != UINT32_MAX;
  }

  static void destroy(uint32_t *ref_count_ptr) {
    if (!is_strong(ref_count_ptr)) {
      --ref_count_ptr;
    }
    delete reinterpret_cast<ControlBlock *>(ref_count_ptr);
  }
};

template <typename T>
concept has_ref_count_field = requires(T obj) {
  { obj.m_ref_count } -> amelia::matches_type<uint32_t *>;
};
} // namespace internal

template <typename T> struct FlexFromThis;

/* Non-threadsafe reference-counted pointer that can be either strong or weak. */
template <typename T> class Flex {
public:
  Flex() noexcept {}

  Flex(const Flex<T> &other) noexcept : m_object(other.m_object), m_ref_count(other.m_ref_count) {
    acquire();
  }

  template <typename U>
  Flex(const Flex<U> &other) noexcept : m_object(other.m_object), m_ref_count(other.m_ref_count) {
    acquire();
  }

  Flex(Flex<T> &&other) noexcept : m_object(other.m_object), m_ref_count(other.m_ref_count) {
    other.m_object = nullptr;
    other.m_ref_count = nullptr;
  }

  template <typename U>
  Flex(Flex<U> &&other) noexcept : m_object(other.m_object), m_ref_count(other.m_ref_count) {
    other.m_object = nullptr;
    other.m_ref_count = nullptr;
  }

  ~Flex() {
    release();
  }

  Flex<T> weak() const noexcept {
    Flex<T> result;
    result.m_object = m_object;
    result.m_ref_count = m_ref_count;
    if (is_strong()) {
      ++result.m_ref_count;
    }
    result.acquire();
    return result;
  }

  Flex<T> strong() const noexcept {
    Flex<T> result;
    result.m_object = m_object;
    result.m_ref_count = m_ref_count;
    if (!is_strong()) {
      --result.m_ref_count;
    }
    result.acquire();
    return result;
  }

  template <typename U> Flex<U> downcast() const noexcept {
    Flex<U> result;
    result.m_object = static_cast<U *>(m_object);
    result.m_ref_count = m_ref_count;
    result.acquire();
    return result;
  }

  bool is_strong() const noexcept {
    return m_ref_count && internal::ControlBlock::is_strong(m_ref_count);
  }

  bool is_null() const noexcept {
    return m_object == nullptr || m_ref_count == nullptr ||
           (!is_strong() && *(m_ref_count - 1) == 0);
  }

  uint64_t hash_code() const noexcept {
    return bit_hash(reinterpret_cast<uintptr_t>(m_object));
  }

  Flex<T> &operator=(const Flex<T> &other) noexcept {
    if (this != &other) {
      release();
      m_object = other.m_object;
      m_ref_count = other.m_ref_count;
      acquire();
    }
    return *this;
  }

  template <typename U> Flex<T> &operator=(const Flex<U> &other) noexcept {
    release();
    m_object = other.m_object;
    m_ref_count = other.m_ref_count;
    acquire();
    return *this;
  }

  Flex<T> &operator=(Flex<T> &&other) noexcept {
    if (this != &other) {
      release();
      m_object = other.m_object;
      m_ref_count = other.m_ref_count;
      other.m_object = nullptr;
      other.m_ref_count = nullptr;
    }
    return *this;
  }

  template <typename U> Flex<T> &operator=(Flex<U> &&other) noexcept {
    release();
    m_object = other.m_object;
    m_ref_count = other.m_ref_count;
    other.m_object = nullptr;
    other.m_ref_count = nullptr;
    return *this;
  }

  T &operator*() {
    return *get();
  }

  const T &operator*() const {
    return *get();
  }

  T *operator->() {
    return get();
  }

  const T *operator->() const {
    return get();
  }

  bool operator==(const Flex<T> &other) const noexcept {
    return m_object == other.m_object;
  }

  bool operator!=(const Flex<T> &other) const noexcept {
    return m_object != other.m_object;
  }

  operator T &() {
    return *get();
  }

  operator const T &() const {
    return *get();
  }

  template <typename U> friend class Flex;
  template <typename U> friend struct FlexFromThis;

  template <typename U>
    requires(!internal::has_ref_count_field<U>)
  friend Flex<U> make_flex(U &&obj);

  template <typename U>
    requires(internal::has_ref_count_field<U>)
  friend Flex<U> make_flex(U &&obj);

  template <typename U, typename... Args>
    requires(!internal::has_ref_count_field<U>)
  friend Flex<U> emplace_flex(Args &&...args);

  template <typename U, typename... Args>
    requires(internal::has_ref_count_field<U>)
  friend Flex<U> emplace_flex(Args &&...args);

private:
  T *m_object = nullptr;
  uint32_t *m_ref_count = nullptr;

  Flex(T *object, uint32_t *ref_count) : m_object(object), m_ref_count(ref_count) {}

  void acquire() noexcept {
    if (m_ref_count) {
      ++(*m_ref_count);
    }
  }

  void release() noexcept {
    if (m_ref_count) {
      if (internal::ControlBlock::is_strong(m_ref_count)) {
        if (*m_ref_count == 1) {
          delete m_object;
          m_object = nullptr;
          if (*(m_ref_count + 1) == 0) {
            internal::ControlBlock::destroy(m_ref_count);
            m_ref_count = nullptr;
          } else {
            --(*m_ref_count);
          }
        } else {
          --(*m_ref_count);
        }
      } else {
        if (
          // if strong count is 0 and weak count is 1
          *(m_ref_count - 1) == 0 && (*m_ref_count) == 1
        ) {
          internal::ControlBlock::destroy(m_ref_count);
          m_ref_count = nullptr;
        } else {
          --(*m_ref_count);
        }
      }
    } else if (m_object) {
      // if we have an object but no refcount, then this destructor is running due to an exception
      // thrown after construction of the object but before construction of the control block
      delete m_object;
    }
  }

  T *get() const {
    if (!m_object || !m_ref_count) {
      throw RuntimeError("Dereferencing null Flex");
    } else if (
      // if the strong count is zero, then the object has already been deleted
      !is_strong() && *(m_ref_count - 1) == 0
    ) {
      throw RuntimeError("Dereferencing expired weak Flex");
    }
    return m_object;
  }
};

template <typename T> struct FlexFromThis {
public:
  Flex<T> flex() const {
    if (!m_ref_count) {
      throw RuntimeError("FlexFromThis::flex() called on an object that was not created with "
                         "make_flex or emplace_flex");
    }
    Flex<T> result;
    result.m_object = const_cast<T *>(static_cast<const T *>(this));
    result.m_ref_count = m_ref_count;
    result.acquire();
    return result;
  }

  template <typename U>
    requires(!internal::has_ref_count_field<U>)
  friend Flex<U> make_flex(U &&obj);

  template <typename U>
    requires(internal::has_ref_count_field<U>)
  friend Flex<U> make_flex(U &&obj);

  template <typename U, typename... Args>
    requires(!internal::has_ref_count_field<U>)
  friend Flex<U> emplace_flex(Args &&...args);

  template <typename U, typename... Args>
    requires(internal::has_ref_count_field<U>)
  friend Flex<U> emplace_flex(Args &&...args);

private:
  uint32_t *m_ref_count = nullptr;
};

template <typename U>
  requires(!internal::has_ref_count_field<U>)
Flex<U> make_flex(U &&obj) {
  Flex<U> result;
  result.m_object = new U(move(obj));
  result.m_ref_count = internal::ControlBlock::make();
  result.acquire();
  return result;
}

template <typename U>
  requires(internal::has_ref_count_field<U>)
Flex<U> make_flex(U &&obj) {
  Flex<U> result;
  result.m_object = new U(move(obj));
  result.m_ref_count = internal::ControlBlock::make();
  result.acquire();
  result.m_object->m_ref_count = result.m_ref_count;
  return result;
}

template <typename U, typename... Args>
  requires(!internal::has_ref_count_field<U>)
Flex<U> emplace_flex(Args &&...args) {
  Flex<U> result;
  result.m_object = new U(amelia::forward<Args>(args)...);
  result.m_ref_count = internal::ControlBlock::make();
  result.acquire();
  return result;
}

template <typename U, typename... Args>
  requires(internal::has_ref_count_field<U>)
Flex<U> emplace_flex(Args &&...args) {
  Flex<U> result;
  result.m_object = new U(amelia::forward<Args>(args)...);
  result.m_ref_count = internal::ControlBlock::make();
  result.acquire();
  result.m_object->m_ref_count = result.m_ref_count;
  return result;
}

} // namespace amelia
