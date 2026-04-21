#include <utfcpp/utf8.h>

#include "CharIterator.h"
#include "InvalidUTF8Error.h"
#include "Text.h"

namespace amelia {

CharIterator::CharIterator(Slice<const char> str) noexcept : slice(str) {}
CharIterator::CharIterator(Text text) noexcept : slice(text.data()) {}

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

CharIterator CharIterator::end() const noexcept { return CharIterator(slice.end()); }

uint32_t CharIterator::operator*() const { return peek(); }

uint32_t CharIterator::peek() const {
  try {
    return utf8::peek_next(slice, slice.end());
  } catch (...) {
    throw amelia::InvalidUTF8Error();
  }
}

uint32_t CharIterator::next() {
  try {
    return utf8::next(slice, slice.end());
  } catch (...) {
    throw amelia::InvalidUTF8Error();
  }
}

CharIterator CharIterator::find(Text substring) const {
  CharIterator start_iter = *this;
  CharIterator current_iter = start_iter;
  CharIterator start_textiter = CharIterator(substring);
  CharIterator current_textiter = start_textiter;
  while (true) {
    if (current_textiter.at_end()) {
      return start_iter;
    }
    if (current_iter.at_end()) {
      return CharIterator(slice.end());
    }
    if (current_iter.peek() == current_textiter.peek()) {
      current_textiter.next();
      current_iter.next();
    } else {
      current_textiter = start_textiter;
      current_iter.next();
      start_iter = current_iter;
    }
  }
}

CharIterator CharIterator::find(uint32_t code_point) const {
  CharIterator current_iter = *this;
  while (!current_iter.at_end()) {
    if (current_iter.peek() == code_point) {
      return current_iter;
    }
    current_iter.next();
  }
  return end();
}

Text CharIterator::head(CharIterator end) const noexcept {
  return Text(Slice(slice.ptr(), end.slice.ptr() - slice.ptr()));
}

Text CharIterator::tail(CharIterator start) const noexcept {
  return Text(Slice(start.slice.ptr(), slice.end().ptr() - start.slice.ptr()));
}

Text CharIterator::subslice(CharIterator start, CharIterator end) const {
  return Text(Slice(start.slice.ptr(), end.slice.ptr() - start.slice.ptr()));
}

bool CharIterator::operator==(const CharIterator &other) const noexcept {
  return slice == other.slice;
}

bool CharIterator::operator!=(const CharIterator &other) const noexcept {
  return !(*this == other);
}

bool CharIterator::at_end() const noexcept { return slice.size() == 0; }

CharIterator::operator bool() const noexcept { return !at_end(); }

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
  if (a.ptr() == b.ptr() && a.size() == b.size()) {
    return 0;
  }

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

  if (self_iter == self_end && other_iter == other_end) {
    return 0;
  }
  return self_iter == self_end ? -1 : 1;
}

} // namespace amelia
