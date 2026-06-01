#pragma once

#include <stdexcept>
#include <utility>

namespace amelia {

struct None {};

template <typename T> struct Some {
  T value;
  Some(T value) : value(std::move(value)) {}
};

template <typename T> class Option {
public:
  Option() {}

  Option(T value) {
    initialize(std::move(value));
  }

  Option(Some<T> some) {
    initialize(std::move(some.value));
  }

  Option(None) {}

  template <typename U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
  Option(U &&value) {
    initialize(static_cast<T>(std::forward<U>(value)));
  }

  template <typename U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
  Option(Some<U> &&some) {
    initialize(static_cast<T>(std::forward<U>(some.value)));
  }

  Option(const Option<T> &other) {
    if (other.m_has_value) {
      initialize(*other.get());
    }
  }

  Option(Option<T> &&other) {
    if (other.m_has_value) {
      initialize(std::move(*other.get()));
    }
    other.m_has_value = false;
  }

  Option<T> &operator=(const Option<T> &other) {
    if (this != &other) {
      if (other.m_has_value) {
        assign(*other.get());
      } else {
        destroy();
      }
    }
    return *this;
  }

  Option<T> &operator=(Option<T> &&other) {
    if (this != &other) {
      if (other.m_has_value) {
        assign(std::move(*other.get()));
      } else {
        destroy();
      }
      other.m_has_value = false;
    }
    return *this;
  }

  template <typename U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
  Option<T> &operator=(U &&value) {
    assign(static_cast<T>(std::forward<U>(value)));
    return *this;
  }

  template <typename U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
  Option<T> &operator=(Some<U> &&some) {
    assign(static_cast<T>(std::forward<U>(some.value)));
    return *this;
  }

  ~Option() {
    destroy();
  }

  bool has_value() const noexcept {
    return m_has_value;
  }

  T &value() {
    if (!m_has_value) {
      throw std::runtime_error("Attempted to access value of an empty Option");
    }
    return *get();
  }

  const T &value() const {
    if (!m_has_value) {
      throw std::runtime_error("Attempted to access value of an empty Option");
    }
    return *get();
  }

  T &operator*() {
    return value();
  }

  const T &operator*() const {
    return value();
  }

  T *operator->() {
    if (!m_has_value) {
      throw std::runtime_error("Attempted to access value of an empty Option");
    }
    return get();
  }

  const T *operator->() const {
    if (!m_has_value) {
      throw std::runtime_error("Attempted to access value of an empty Option");
    }
    return get();
  }

  void clear() {
    destroy();
  }

  template <typename U, typename = std::enable_if_t<std::is_convertible_v<T, U>>>
  operator Option<U>() const {
    if (m_has_value) {
      return Some(static_cast<U>(value()));
    }
    return None();
  }

  bool operator==(const Option<T> &other) const {
    if (m_has_value != other.m_has_value) {
      return false;
    }
    if (m_has_value) {
      return value() == other.value();
    }
    return true;
  }

  bool operator!=(const Option<T> &other) const {
    return !(*this == other);
  }

private:
  union Container {
    T value;
    char storage[sizeof(T)];
    ~Container() {}
  };

  void initialize(T value) {
    new (get()) T(std::move(value));
    m_has_value = true;
  }

  void assign(T value) {
    if (m_has_value) {
      *get() = std::move(value);
    } else {
      initialize(std::move(value));
    }
  }

  void destroy() {
    if (m_has_value) {
      m_has_value = false;
      get()->~T();
    }
  }

  T *get() {
    return reinterpret_cast<T *>(&m_container.storage);
  }
  const T *get() const {
    return reinterpret_cast<const T *>(&m_container.storage);
  }

  bool m_has_value = false;
  Container m_container{.storage = {}};
};

} // namespace amelia
