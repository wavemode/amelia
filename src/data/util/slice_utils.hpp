#include "prelude.hpp"

namespace amelia {

struct SliceUtils {

  template <typename T> static Slice<T> subseq(Slice<T> slice, size_t start, size_t end) {
    if (start > end) {
      throw RuntimeError("Start index cannot be greater than end index");
    }
    if (start > slice.size()) {
      start = slice.size();
    }
    if (end > slice.size()) {
      end = slice.size();
    }
    return Slice<T>(slice.ptr() + start, end - start);
  }

  template <typename T> static Slice<T> slice(Slice<T> slice, size_t start, size_t count) {
    if (start > slice.size()) {
      start = slice.size();
    }
    if (start + count > slice.size()) {
      count = slice.size() - start;
    }
    return Slice<T>(slice.ptr(), count);
  }

  template <typename T> static Slice<T> tail(Slice<T> slice, size_t count) {
    if (count > slice.size()) {
      count = slice.size();
    }
    return Slice<T>(slice.ptr() + slice.size() - count, count);
  }

  template <typename T> static Slice<T> head(Slice<T> slice, size_t count) {
    if (count > slice.size()) {
      count = slice.size();
    }
    return Slice<T>(slice.ptr(), count);
  }
};

} // namespace amelia
