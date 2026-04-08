#include <vendor/utfcpp/utf8.h>

#include "CharIterator.h"
#include "InvalidUTF8Error.h"

amelia::CharIterator::CharIterator(Slice<const char> str) : current(str.begin()), end(str.end()) {}

uint32_t amelia::CharIterator::operator++() {
  try {
    return utf8::next(current, end);
  } catch (...) {
    throw amelia::InvalidUTF8Error();
  }
}

uint32_t amelia::CharIterator::operator*() const {
  try {
    return utf8::peek_next(current, end);
  } catch (...) {
    throw amelia::InvalidUTF8Error();
  }
}

bool amelia::CharIterator::operator==(const CharIterator &other) const noexcept {
  return current == other.current;
}

bool amelia::CharIterator::operator!=(const CharIterator &other) const noexcept {
  return !(*this == other);
}

bool amelia::CharIterator::at_end() const noexcept { return current == end; }

void amelia::CharIterator::validate(Slice<const char> str) {
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

void amelia::CharIterator::append(uint32_t code_point, std::string &str) {
  try {
    utf8::append(code_point, std::back_inserter(str));
  } catch (...) {
    throw amelia::InvalidUTF8Error();
  }
}
