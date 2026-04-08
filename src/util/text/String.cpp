#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

#include "CharIterator.h"
#include "String.h"

namespace amelia {

String::String() noexcept = default;

String::String(const char *str) : data(str) { CharIterator::validate(str, str + std::strlen(str)); }

const char *String::c_str() const noexcept { return data.c_str(); }

size_t String::size() const noexcept { return data.size(); }

void String::append(const char *str) {
  CharIterator::validate(str, str + std::strlen(str));
  data.append(str);
}

void String::append(const String &other) { append(other.c_str()); }

void String::append(uint32_t code_point) { CharIterator::append(code_point, data); }

CharIterator String::begin() const {
  return CharIterator(data.c_str(), data.c_str() + data.size());
}

CharIterator String::end() const {
  const char *str_end = c_str() + size();
  return CharIterator(str_end, str_end);
}

String String::operator+(const String &other) const {
  String result(*this);
  result.append(other);
  return result;
}

bool String::operator==(const String &other) const { return data == other.data; }

bool String::operator!=(const String &other) const { return !(*this == other); }

bool String::operator<(const String &other) const {
  auto self_iter = begin();
  auto self_end = end();
  auto other_iter = other.begin();
  auto other_end = other.end();
  while (self_iter != self_end && other_iter != other_end) {
    if (*self_iter < *other_iter) {
      return true;
    } else if (*self_iter > *other_iter) {
      return false;
    }
    ++self_iter;
    ++other_iter;
  }
  return self_iter == self_end && other_iter != other_end;
}

bool String::operator<=(const String &other) const {
  auto self_iter = begin();
  auto self_end = end();
  auto other_iter = other.begin();
  auto other_end = other.end();
  while (self_iter != self_end && other_iter != other_end) {
    if (*self_iter < *other_iter) {
      return true;
    } else if (*self_iter > *other_iter) {
      return false;
    }
    ++self_iter;
    ++other_iter;
  }
  // at this point, either both iterators are at their end (which means the Strings are equal),
  // or one of them is not (which means it is not less than the other String)
  return self_iter == self_end;
}

bool String::operator>(const String &other) const { return !(*this <= other); }

bool String::operator>=(const String &other) const { return !(*this < other); }

} // namespace amelia
