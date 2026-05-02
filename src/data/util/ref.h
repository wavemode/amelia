#pragma once

namespace amelia {

template <typename T> class Ref {
public:
  Ref(T &value) : m_ref(&value) {}

  T &get() { return *m_ref; }
  const T &get() const { return *m_ref; }

  T &operator*() { return get(); }
  const T &operator*() const { return get(); }
  T *operator->() { return m_ref; }
  const T *operator->() const { return m_ref; }

  operator T &() { return get(); }
  operator const T &() const { return get(); }

private:
  T *m_ref;
};

template <typename T> class ConstRef {
public:
  ConstRef(const T &value) : m_ref(&value) {}

  const T &get() const { return *m_ref; }

  const T &operator*() const { return get(); }

  const T *operator->() const { return m_ref; }

  operator const T &() const { return get(); }

private:
  const T *m_ref;
};

} // namespace amelia
