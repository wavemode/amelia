#include <utfcpp/utf8.h>

#include "CharIterator.h"
#include "InvalidUTF8Error.h"

namespace amelia {

CharIterator::CharIterator(Slice<const char> str) noexcept : current(str.begin()), end(str.end()) {}

CharIterator &CharIterator::operator++() {
  next();
  return *this;
}

CharIterator CharIterator::operator++(int) {
  CharIterator temp = *this;
  ++(*this);
  return temp;
}

uint32_t CharIterator::operator*() const { return peek(); }

uint32_t CharIterator::peek() const {
  try {
    return utf8::peek_next(current, end);
  } catch (...) {
    throw amelia::InvalidUTF8Error();
  }
}

uint32_t CharIterator::next() {
  try {
    return utf8::next(current, end);
  } catch (...) {
    throw amelia::InvalidUTF8Error();
  }
}

bool CharIterator::operator==(const CharIterator &other) const noexcept {
  return current == other.current;
}

bool CharIterator::operator!=(const CharIterator &other) const noexcept {
  return !(*this == other);
}

bool CharIterator::at_end() const noexcept { return current == end; }

void CharIterator::validate(Slice<const char> str) {
  auto begin = str.begin();
  auto end = str.end();
  while (begin != end) {
    try {
      utf8::next(begin, end);
    } catch (...) {
      throw amelia::InvalidUTF8Error();
    }
  }
}

void CharIterator::append(uint32_t code_point, std::string &str) {
  try {
    utf8::append(code_point, std::back_inserter(str));
  } catch (...) {
    throw amelia::InvalidUTF8Error();
  }
}

signed char CharIterator::compare(Slice<const char> a, Slice<const char> b) {
  auto self_iter = CharIterator(a);
  auto self_end = CharIterator(a + a.size());
  auto other_iter = CharIterator(b);
  auto other_end = CharIterator(b + b.size());
  while (self_iter != self_end && other_iter != other_end) {
    uint32_t self_cp = self_iter.next();
    uint32_t other_cp = other_iter.next();
    if (self_cp < other_cp) {
      return -1;
    } else if (self_cp > other_cp) {
      return 1;
    }
  }
  if (self_iter == self_end && other_iter == other_end)
    return 0;
  return self_iter == self_end ? -1 : 1;
}

} // namespace amelia
