#pragma once

#include <algorithm>
#include <cstddef>

#include "List.h"
#include "Slice.h"

namespace amelia {

struct SliceUtils {
  template <typename T> static void sort(IList<T> &list) { sort(Slice<T>(list)); }

  template <typename T> static void sort(Slice<T> list) {
    std::sort(list.ptr(), list.ptr() + list.size());
  }

  template <typename T, typename CompareFn> static void sort(IList<T> &list, CompareFn comp) {
    sort(Slice<T>(list), comp);
  }

  template <typename T, typename CompareFn> static void sort(Slice<T> list, CompareFn comp) {
    std::sort(list.ptr(), list.ptr() + list.size(), comp);
  }

  template <typename T> static void reverse(IList<T> &list) { reverse(Slice<T>(list)); }

  template <typename T> static void reverse(Slice<T> list) {
    std::reverse(list.ptr(), list.ptr() + list.size());
  }
};

} // namespace amelia
