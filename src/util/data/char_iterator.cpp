#include <cstring>

#include <utfcpp/utf8.h>

#include "char_iterator.hpp"

#include "util/data/invalid_utf8_error.hpp"
#include "util/data/list.hpp"
#include "util/data/text.hpp"
#include "util/data/utility.hpp"

namespace amelia {

CharIterator::CharIterator(ConstSliceIterator<char> str) noexcept : m_slice_iter(str) {}
CharIterator::CharIterator(Text text) noexcept : m_slice_iter(text.data()) {}

CharIterator &CharIterator::operator++() {
  next();
  return *this;
}

CharIterator CharIterator::operator++(int) {
  CharIterator temp = *this;
  ++(*this);
  return temp;
}

CharIterator CharIterator::begin() const noexcept {
  return *this;
}

CharIterator CharIterator::end() const noexcept {
  return CharIterator(m_slice_iter.end());
}

uint32_t CharIterator::operator*() {
  return peek();
}

uint32_t CharIterator::peek() {
  try {
    return utf8::peek_next(m_slice_iter, m_slice_iter.end());
  } catch (const utf8::invalid_utf8 &) {
    throw amelia::InvalidUTF8Error();
  } catch (const utf8::not_enough_room &) {
    throw RuntimeError("Attempted to peek past the end of the string");
  }
}

uint32_t CharIterator::next() {
  try {
    return utf8::next(m_slice_iter, m_slice_iter.end());
  } catch (const utf8::invalid_utf8 &) {
    throw amelia::InvalidUTF8Error();
  } catch (const utf8::not_enough_room &) {
    throw RuntimeError("Attempted to advance past the end of the string");
  }
}

ConstSlice<char> CharIterator::data() const noexcept {
  return ConstSlice(m_slice_iter);
}

Text CharIterator::text() const noexcept {
  return Text(data());
}

CharIterator CharIterator::plus(size_t n) const {
  CharIterator iter = *this;
  for (size_t i = 0; i < n; ++i) {
    iter.next();
  }
  return iter;
}

CharIterator CharIterator::plus_bytes(size_t n) const {
  CharIterator iter = *this;
  iter.m_slice_iter = iter.m_slice_iter + n;
  return iter;
}

bool CharIterator::operator==(const CharIterator &other) const noexcept {
  return m_slice_iter == other.m_slice_iter;
}

bool CharIterator::operator!=(const CharIterator &other) const noexcept {
  return !(*this == other);
}

bool CharIterator::at_end() const noexcept {
  return m_slice_iter.size() == 0;
}

void CharIterator::validate(ConstSlice<char> str) {
  auto begin = str.begin();
  auto end = str.end();
  while (begin != end) {
    try {
      utf8::next(begin, end);
    } catch (const utf8::invalid_utf8 &) {
      throw amelia::InvalidUTF8Error();
    } catch (const utf8::not_enough_room &) {
      throw amelia::InvalidUTF8Error();
    }
  }
}

void CharIterator::append(List<char> &str, uint32_t code_point) {
  try {
    utf8::append(code_point, std::back_inserter(str));
  } catch (const utf8::invalid_code_point &) {
    throw amelia::InvalidUTF8Error();
  }
}

signed char CharIterator::compare(ConstSlice<char> a, ConstSlice<char> b) {
  if (a.ptr() == nullptr) {
    return b.size() == 0 ? 0 : -1;
  } else if (b.ptr() == nullptr) {
    return a.size() == 0 ? 0 : 1;
  }
  int cmp = (a.ptr() == b.ptr()) ? 0
                                 : std::memcmp(a.ptr(), b.ptr(), amelia::min(a.size(), b.size()));
  if (cmp == 0) {
    if (a.size() < b.size()) {
      return -1;
    } else if (a.size() > b.size()) {
      return 1;
    } else {
      return 0;
    }
  } else if (cmp < 0) {
    return -1;
  } else {
    return 1;
  }
}

bool CharIterator::is_valid_code_point(uint32_t cp) {
  return utf8::internal::is_code_point_valid(cp);
}
} // namespace amelia
