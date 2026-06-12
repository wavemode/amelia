#pragma once

#include "data/util/utility.hpp"

namespace amelia {

template <typename T> class Box {
public:
  template <typename... Args> static Box<T> emplace(Args &&...args) {
    return Box<T>(T(amelia::forward<Args>(args)...));
  }

  Box() : m_obj(nullptr) {}

  explicit Box(T obj) : m_obj(new T(move(obj))) {}

  Box(const Box<T> &other) = delete;

  Box(Box<T> &&other) noexcept : m_obj(other.m_obj) {
    other.m_obj = nullptr;
  }

  template <typename U> Box(Box<U> &&other) noexcept : m_obj(other.m_obj) {
    other.m_obj = nullptr;
  }

  ~Box() {
    delete m_obj;
  }

  bool is_null() const {
    return m_obj == nullptr;
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

  T &operator*() {
    return *m_obj;
  }

  const T &operator*() const {
    return *m_obj;
  }

  T *operator->() {
    return m_obj;
  }

  const T *operator->() const {
    return m_obj;
  }

  template <typename U> friend class Box;

private:
  T *m_obj;
};

} // namespace amelia
