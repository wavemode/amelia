#pragma once

#include "data/util/utility.hpp"

namespace amelia {

namespace internal {
template <typename T> struct SharedObject {
  T obj;
  uint32_t ref_count;
};
} // namespace internal

/*
  Reference-counted pointer that can be either strong or weak. There is no separate control block,
  so the weak pointers are not safe to use except in the internals of the typechecker, where we
  occasionally create weak pointers when both of the following are true:

  1) Due to the structure of the data, the object we're pointing to is guaranteed to have some
     strong pointer pointing to it at all times.

  2) Using a strong pointer would risk creating a memory leak due to reference cycles.

  The use of shared pointers in the typechecker is actually an optimization, as it allows the
  caching system to reuse entire type trees rather than reconstructing them.
*/
template <typename T> class FlexShared {
public:
  static FlexShared<T> strong(T &&obj) {
    auto *shared_obj = new internal::SharedObject<T>{move(obj), 0};
    return FlexShared<T>(shared_obj, true);
  }

  FlexShared<T> weak() const {
    return FlexShared<T>(m_obj, false);
  }

  FlexShared(const FlexShared<T> &other) : m_obj(other.m_obj), m_strong(other.m_strong) {
    if (m_strong) {
      m_obj->ref_count++;
    }
  }

  FlexShared(FlexShared<T> &&other) noexcept : m_obj(other.m_obj), m_strong(other.m_strong) {
    other.m_obj = nullptr;
    other.m_strong = false;
  }

  ~FlexShared() {
    release();
  }

  FlexShared<T> &operator=(const FlexShared<T> &other) {
    if (this != &other) {
      release();
      m_obj = other.m_obj;
      m_strong = other.m_strong;
      if (m_strong) {
        m_obj->ref_count++;
      }
    }
    return *this;
  }

  FlexShared<T> &operator=(FlexShared<T> &&other) noexcept {
    if (this != &other) {
      release();
      m_obj = other.m_obj;
      m_strong = other.m_strong;
      other.m_obj = nullptr;
      other.m_strong = false;
    }
    return *this;
  }

  T &operator*() {
    return m_obj->obj;
  }

  const T &operator*() const {
    return m_obj->obj;
  }

  T *operator->() {
    return &m_obj->obj;
  }

  const T *operator->() const {
    return &m_obj->obj;
  }

private:
  FlexShared(internal::SharedObject<T> *obj, bool strong) : m_obj(obj), m_strong(strong) {
    if (m_strong) {
      m_obj->ref_count++;
    }
  }

  void release() {
    if (m_strong && m_obj) {
      m_obj->ref_count--;
      if (m_obj->ref_count == 0) {
        delete m_obj;
      }
    }
    m_obj = nullptr;
    m_strong = false;
  }

  internal::SharedObject<T> *m_obj;
  bool m_strong;
};

} // namespace amelia
