#pragma once

#include "data/util/utility.hpp"

namespace amelia {

/*
  Reference-counted pointer that can be either strong or weak. There is no separate control block,
  so the weak pointers are not safe to use except in the internals of the typechecker, where we
  sometimes create weak pointers to global constants, to save allocations/refcounting for common
  builtin types. We also occasionally create weak pointers when both of the following are true:

  1) Due to the structure of the data, the object we're pointing to is guaranteed to have some
     strong pointer pointing to it at all times.

  2) Using a strong pointer would risk creating a memory leak due to reference cycles.

  The use of shared pointers in the typechecker is actually an optimization, as it allows the
  caching system to reuse entire type trees rather than reconstructing them.
*/
template <typename T> class FlexShared {
public:
  static FlexShared<T> strong(T &&obj) {
    return FlexShared<T>(new T(move(obj)), new volatile uint32_t(1));
  }

  static FlexShared<T> weak(T *obj) {
    return FlexShared<T>(obj, nullptr);
  }

  template <typename... Args> static FlexShared<T> emplace(Args &&...args) {
    return FlexShared<T>(new T(amelia::forward<Args>(args)...), new volatile uint32_t(1));
  }

  FlexShared() noexcept : m_obj(nullptr), m_ref_count(nullptr) {}

  FlexShared(const FlexShared<T> &other) noexcept
      : m_obj(other.m_obj), m_ref_count(other.m_ref_count) {
    acquire();
  }

  template <typename U>
  FlexShared(const FlexShared<U> &other) noexcept
      : m_obj(other.m_obj), m_ref_count(other.m_ref_count) {
    acquire();
  }

  FlexShared(FlexShared<T> &&other) noexcept : m_obj(other.m_obj), m_ref_count(other.m_ref_count) {
    other.m_obj = nullptr;
    other.m_ref_count = nullptr;
  }

  template <typename U>
  FlexShared(FlexShared<U> &&other) noexcept : m_obj(other.m_obj), m_ref_count(other.m_ref_count) {
    other.m_obj = nullptr;
    other.m_ref_count = nullptr;
  }

  ~FlexShared() {
    release();
  }

  FlexShared<T> weak() const noexcept {
    return FlexShared<T>(m_obj, nullptr);
  }

  FlexShared<T> &operator=(const FlexShared<T> &other) noexcept {
    if (this != &other) {
      release();
      m_obj = other.m_obj;
      m_ref_count = other.m_ref_count;
      acquire();
    }
    return *this;
  }

  template <typename U> FlexShared<T> &operator=(const FlexShared<U> &other) noexcept {
    release();
    m_obj = other.m_obj;
    m_ref_count = other.m_ref_count;
    acquire();
    return *this;
  }

  FlexShared<T> &operator=(FlexShared<T> &&other) noexcept {
    if (this != &other) {
      release();
      m_obj = other.m_obj;
      m_ref_count = other.m_ref_count;
      other.m_obj = nullptr;
      other.m_ref_count = nullptr;
    }
    return *this;
  }

  template <typename U> FlexShared<T> &operator=(FlexShared<U> &&other) noexcept {
    release();
    m_obj = other.m_obj;
    m_ref_count = other.m_ref_count;
    other.m_obj = nullptr;
    other.m_ref_count = nullptr;
    return *this;
  }

  T &operator*() noexcept {
    return *m_obj;
  }

  const T &operator*() const noexcept {
    return *m_obj;
  }

  T *operator->() noexcept {
    return m_obj;
  }

  const T *operator->() const noexcept {
    return m_obj;
  }

  template <typename U> friend class FlexShared;

  bool operator==(const FlexShared<T> &other) const noexcept {
    return m_obj == other.m_obj;
  }

  operator T &() noexcept {
    return *m_obj;
  }

  operator const T &() const noexcept {
    return *m_obj;
  }

private:
  FlexShared(T *obj, volatile uint32_t *ref_count) noexcept : m_obj(obj), m_ref_count(ref_count) {}

  void acquire() noexcept {
    if (m_ref_count) {
      __atomic_fetch_add(m_ref_count, 1, __ATOMIC_ACQUIRE);
    }
  }

  void release() noexcept {
    if (m_ref_count) {
      if (__atomic_sub_fetch(m_ref_count, 1, __ATOMIC_RELEASE) == 0) {
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
        delete m_obj;
        delete m_ref_count;
      }
    }
    m_obj = nullptr;
    m_ref_count = nullptr;
  }

  T *m_obj;
  volatile uint32_t *m_ref_count;
};

template <typename T> FlexShared<T> make_flex(T &&obj) {
  return FlexShared<T>::strong(move(obj));
}

template <typename T, typename... Args> FlexShared<T> emplace_flex(Args &&...args) {
  return FlexShared<T>::emplace(amelia::forward<Args>(args)...);
}

} // namespace amelia
