#pragma once

#include "util/data/runtime_error.hpp"
#include "util/data/utility.hpp"

namespace amelia {

namespace internal {
struct ControlBlock {
  uint32_t strong_count;
  uint32_t weak_count;
};

template <typename T>
concept has_control_block_field = requires(T obj) {
  { obj.m_control_block } -> amelia::matches_type<ControlBlock *>;
};
} // namespace internal

template <typename T> struct FlexFromThis;

/* Non-threadsafe reference-counted pointer that can be either strong or weak. */
template <typename T> class Flex {
public:
  Flex() noexcept {}

  Flex(const Flex<T> &other) noexcept
      : m_object(other.m_object), m_control_block(other.m_control_block),
        m_is_strong(other.m_is_strong) {
    acquire();
  }

  template <typename U>
  Flex(const Flex<U> &other) noexcept
      : m_object(other.m_object), m_control_block(other.m_control_block),
        m_is_strong(other.m_is_strong) {
    acquire();
  }

  Flex(Flex<T> &&other) noexcept
      : m_object(other.m_object), m_control_block(other.m_control_block),
        m_is_strong(other.m_is_strong) {
    other.m_object = nullptr;
    other.m_control_block = nullptr;
  }

  template <typename U>
  Flex(Flex<U> &&other) noexcept
      : m_object(other.m_object), m_control_block(other.m_control_block),
        m_is_strong(other.m_is_strong) {
    other.m_object = nullptr;
    other.m_control_block = nullptr;
  }

  ~Flex() {
    release();
  }

  Flex<T> weak() const noexcept {
    Flex<T> result;
    result.m_object = m_object;
    result.m_control_block = m_control_block;
    result.m_is_strong = false;
    result.acquire();
    return result;
  }

  Flex<T> strong() const noexcept {
    Flex<T> result;
    result.m_object = m_object;
    result.m_control_block = m_control_block;
    result.m_is_strong = true;
    result.acquire();
    return result;
  }

  template <typename U> Flex<U> downcast() const noexcept {
    Flex<U> result;
    result.m_object = static_cast<U *>(m_object);
    result.m_control_block = m_control_block;
    result.m_is_strong = m_is_strong;
    result.acquire();
    return result;
  }

  bool is_null() const noexcept {
    return m_object == nullptr || m_control_block == nullptr || m_control_block->strong_count == 0;
  }

  Flex<T> &operator=(const Flex<T> &other) noexcept {
    if (this != &other) {
      release();
      m_object = other.m_object;
      m_control_block = other.m_control_block;
      m_is_strong = other.m_is_strong;
      acquire();
    }
    return *this;
  }

  template <typename U> Flex<T> &operator=(const Flex<U> &other) noexcept {
    release();
    m_object = other.m_object;
    m_control_block = other.m_control_block;
    m_is_strong = other.m_is_strong;
    acquire();
    return *this;
  }

  Flex<T> &operator=(Flex<T> &&other) noexcept {
    if (this != &other) {
      release();
      m_object = other.m_object;
      m_control_block = other.m_control_block;
      m_is_strong = other.m_is_strong;
      other.m_object = nullptr;
      other.m_control_block = nullptr;
    }
    return *this;
  }

  template <typename U> Flex<T> &operator=(Flex<U> &&other) noexcept {
    release();
    m_object = other.m_object;
    m_control_block = other.m_control_block;
    m_is_strong = other.m_is_strong;
    other.m_object = nullptr;
    other.m_control_block = nullptr;
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
    return m_control_block == other.m_control_block && m_object == other.m_object;
  }

  bool operator!=(const Flex<T> &other) const noexcept {
    return !(*this == other);
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
    requires(!internal::has_control_block_field<U>)
  friend Flex<U> make_flex(U &&obj);

  template <typename U>
    requires(internal::has_control_block_field<U>)
  friend Flex<U> make_flex(U &&obj);

  template <typename U, typename... Args>
    requires(!internal::has_control_block_field<U>)
  friend Flex<U> emplace_flex(Args &&...args);

  template <typename U, typename... Args>
    requires(internal::has_control_block_field<U>)
  friend Flex<U> emplace_flex(Args &&...args);

private:
  Flex(T *object, internal::ControlBlock *control_block, bool is_strong)
      : m_object(object), m_control_block(control_block), m_is_strong(is_strong) {}

  void acquire() noexcept {
    if (m_control_block) {
      if (m_is_strong) {
        ++m_control_block->strong_count;
      } else {
        ++m_control_block->weak_count;
      }
    }
  }

  void release() noexcept {
    if (m_control_block) {
      if (m_is_strong) {
        if (m_control_block->strong_count == 1) {
          delete m_object;
          m_object = nullptr;
          if (m_control_block->weak_count == 0) {
            delete m_control_block;
            m_control_block = nullptr;
          } else {
            --m_control_block->strong_count;
          }
        } else {
          --m_control_block->strong_count;
        }
      } else {
        if (m_control_block->strong_count == 0 && m_control_block->weak_count == 1) {
          delete m_control_block;
          m_control_block = nullptr;
        } else {
          --m_control_block->weak_count;
        }
      }
    }
  }

  T *get() const {
    if (!m_object || !m_control_block) {
      throw RuntimeError("Dereferencing null Flex");
    } else if (m_control_block->strong_count == 0) {
      throw RuntimeError("Dereferencing expired weak Flex");
    }
    return m_object;
  }

  T *m_object = nullptr;
  internal::ControlBlock *m_control_block = nullptr;
  bool m_is_strong = false;
};

template <typename T> struct FlexFromThis {
public:
  Flex<T> flex() const {
    if (!m_control_block) {
      throw RuntimeError("FlexFromThis::flex() called on an object that was not created with "
                         "make_flex or emplace_flex");
    }
    Flex<T> result;
    result.m_object = const_cast<T *>(static_cast<const T *>(this));
    result.m_control_block = m_control_block;
    result.m_is_strong = true;
    result.acquire();
    return result;
  }

  template <typename U>
    requires(!internal::has_control_block_field<U>)
  friend Flex<U> make_flex(U &&obj);

  template <typename U>
    requires(internal::has_control_block_field<U>)
  friend Flex<U> make_flex(U &&obj);

  template <typename U, typename... Args>
    requires(!internal::has_control_block_field<U>)
  friend Flex<U> emplace_flex(Args &&...args);

  template <typename U, typename... Args>
    requires(internal::has_control_block_field<U>)
  friend Flex<U> emplace_flex(Args &&...args);

private:
  internal::ControlBlock *m_control_block = nullptr;
};

template <typename U>
  requires(!internal::has_control_block_field<U>)
Flex<U> make_flex(U &&obj) {
  Flex<U> result;
  result.m_object = new U(move(obj));
  result.m_control_block = new internal::ControlBlock{1, 0};
  result.m_is_strong = true;
  return result;
}

template <typename U>
  requires(internal::has_control_block_field<U>)
Flex<U> make_flex(U &&obj) {
  Flex<U> result;
  result.m_object = new U(move(obj));
  result.m_control_block = new internal::ControlBlock{1, 0};
  result.m_is_strong = true;
  result.m_object->m_control_block = result.m_control_block;
  return result;
}

template <typename U, typename... Args>
  requires(!internal::has_control_block_field<U>)
Flex<U> emplace_flex(Args &&...args) {
  Flex<U> result;
  result.m_object = new U(amelia::forward<Args>(args)...);
  result.m_control_block = new internal::ControlBlock{1, 0};
  result.m_is_strong = true;
  return result;
}

template <typename U, typename... Args>
  requires(internal::has_control_block_field<U>)
Flex<U> emplace_flex(Args &&...args) {
  Flex<U> result;
  result.m_object = new U(amelia::forward<Args>(args)...);
  result.m_control_block = new internal::ControlBlock{1, 0};
  result.m_is_strong = true;
  result.m_object->m_control_block = result.m_control_block;
  return result;
}

} // namespace amelia
