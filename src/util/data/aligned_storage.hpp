#pragma once

namespace amelia {

template <typename T> union AlignedStorage {
  T value;
  char storage[sizeof(T)];
  AlignedStorage() : storage() {}
  ~AlignedStorage() {}

  T *get() { return reinterpret_cast<T *>(&storage); }
  const T *get() const { return reinterpret_cast<const T *>(&storage); }
};

} // namespace amelia