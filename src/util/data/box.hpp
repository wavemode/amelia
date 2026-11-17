#pragma once

#include "util/data/utility.hpp"

namespace amelia {

template <typename T> class Box {
public:
  explicit Box(T obj) : m_obj(new T(move(obj))) {}

  template <typename... Args> static Box<T> emplace(Args &&...args) {
    return Box(new T(amelia::forward<Args>(args)...));
  }

  Box(const Box<T> &other) = delete;

  Box(Box<T> &&other) noexcept : m_obj(other.m_obj) { other.m_obj = nullptr; }

  template <typename U> Box(Box<U> &&other) noexcept : m_obj(other.m_obj) {
    other.m_obj = nullptr;
  }

  ~Box() { delete m_obj; }

  bool is_null() const { return m_obj == nullptr; }

  template <typename U> Box<U> move_as() {
    Box<U> box(nullptr);
    box.m_obj = static_cast<U *>(m_obj);
    m_obj = nullptr;
    return box;
  }

  Box<T> &operator=(const Box<T> &other) = delete;

  Box<T> &operator=(Box<T> &&other) noexcept {
    if (this != &other) {
      delete m_obj;
      m_obj = other.m_obj;
      other.m_obj = nullptr;
    }
    return *this;
  }

  template <typename U> Box<T> &operator=(Box<U> &&other) noexcept {
    delete m_obj;
    m_obj = other.m_obj;
    other.m_obj = nullptr;
    return *this;
  }

  T &operator*() { return *get(); }

  const T &operator*() const { return *get(); }

  T *operator->() { return get(); }

  const T *operator->() const { return get(); }

  template <typename U> friend class Box;

private:
  Box(T *obj) : m_obj(obj) {}

  T *get() const {
    if (m_obj == nullptr) {
      throw_runtime_error("Attempted to dereference null Box");
    }
    return m_obj;
  }

  T *m_obj;
};

} // namespace amelia
