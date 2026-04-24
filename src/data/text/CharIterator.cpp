#include "Prelude.h"

#include <stdexcept>
#include <utfcpp/utf8.h>

namespace amelia {

CharIterator::CharIterator(SliceIterator<const char> str) noexcept : slice_iter(str) {}
CharIterator::CharIterator(Text text) noexcept : slice_iter(text.data()) {}

CharIterator &CharIterator::operator++() {
  next();
  return *this;
}

CharIterator CharIterator::operator++(int) {
  CharIterator temp = *this;
  ++(*this);
  return temp;
}

CharIterator CharIterator::begin() const noexcept { return *this; }

CharIterator CharIterator::end() const noexcept { return CharIterator(slice_iter.end()); }

uint32_t CharIterator::operator*() const { return peek(); }

uint32_t CharIterator::peek() const {
  try {
    return utf8::peek_next(slice_iter, slice_iter.end());
  } catch (const utf8::invalid_utf8 &) {
    throw amelia::InvalidUTF8Error();
  } catch (const utf8::not_enough_room &) {
    throw RuntimeError("Attempted to peek past the end of the string");
  }
}

uint32_t CharIterator::next() {
  try {
    return utf8::next(slice_iter, slice_iter.end());
  } catch (const utf8::invalid_utf8 &) {
    throw amelia::InvalidUTF8Error();
  } catch (const utf8::not_enough_room &) {
    throw RuntimeError("Attempted to advance past the end of the string");
  }
}

Slice<const char> CharIterator::data() const noexcept { return Slice(slice_iter); }

CharIterator CharIterator::plus(size_t n) const {
  CharIterator iter = *this;
  for (size_t i = 0; i < n; ++i) {
    iter.next();
  }
  return iter;
}

CharIterator CharIterator::plus_bytes(size_t n) const {
  CharIterator iter = *this;
  iter.slice_iter = iter.slice_iter + n;
  return iter;
}

bool CharIterator::operator==(const CharIterator &other) const noexcept {
  return slice_iter == other.slice_iter;
}

bool CharIterator::operator!=(const CharIterator &other) const noexcept {
  return !(*this == other);
}

bool CharIterator::at_end() const noexcept { return slice_iter.size() == 0; }

void CharIterator::validate(Slice<const char> str) {
  auto begin = str.begin();
  auto end = str.end();
  while (begin != end) {
    try {
      utf8::next(begin, end);
    } catch (const utf8::invalid_utf8 &) {
      throw amelia::InvalidUTF8Error();
    }
  }
}

void CharIterator::append(uint32_t code_point, std::string &str) {
  try {
    utf8::append(code_point, std::back_inserter(str));
  } catch (const utf8::invalid_code_point &) {
    throw amelia::InvalidUTF8Error();
  }
}

signed char CharIterator::compare(Slice<const char> a, Slice<const char> b) {
  if (a.ptr() == b.ptr() && a.size() == b.size()) {
    return 0;
  }

  auto self_iter = CharIterator(a);
  auto other_iter = CharIterator(b);

  while (!self_iter.at_end() && !other_iter.at_end()) {
    uint32_t self_cp = self_iter.next();
    uint32_t other_cp = other_iter.next();
    if (self_cp < other_cp) {
      return -1;
    } else if (self_cp > other_cp) {
      return 1;
    }
  }

  if (self_iter.at_end() && other_iter.at_end()) {
    return 0;
  }
  return self_iter.at_end() ? -1 : 1;
}

} // namespace amelia
