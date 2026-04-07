#include "CharIterator.h"
#include "InvalidUTF8Error.h"

#include <vendor/utfcpp/utf8.h>

amelia::CharIterator::CharIterator(const char *str) : current(str) {}

uint32_t amelia::CharIterator::operator++() {
  try {
    return utf8::next(current, (const char *)nullptr);
  } catch (...) {
    throw amelia::InvalidUTF8Error();
  }
}

uint32_t amelia::CharIterator::operator*() const {
  try {
    return utf8::peek_next(current, (const char *)nullptr);
  } catch (...) {
    throw amelia::InvalidUTF8Error();
  }
}

bool amelia::CharIterator::operator==(const CharIterator &other) const {
  return current == other.current;
}

bool amelia::CharIterator::operator!=(const CharIterator &other) const { return !(*this == other); }

void amelia::CharIterator::validate(const char *str) {
  while (*str != '\0') {
    try {
      utf8::next(str, (const char *)nullptr);
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
