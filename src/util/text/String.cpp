#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

#include "CharIterator.h"
#include "String.h"

namespace amelia {

String::String() noexcept = default;

String::String(Slice<const char> str) {
  CharIterator::validate(str);
  if (str.size() > 0)
    data.assign(&*str.begin(), str.size());
}

const char *String::c_str() const noexcept { return data.c_str(); }

size_t String::size() const noexcept { return data.size(); }

String &String::append(Slice<const char> str) {
  CharIterator::validate(str);
  if (str.size() > 0)
    data.append(&*str.begin(), str.size());
  return *this;
}

String &String::append(const String &other) {
  data.append(other.data);
  return *this;
}

String &String::append(uint32_t code_point) {
  CharIterator::append(code_point, data);
  return *this;
}

CharIterator String::begin() const {
  return CharIterator(Slice<const char>(data.data(), data.size()));
}

CharIterator String::end() const {
  return CharIterator(Slice<const char>(data.data() + data.size(), 0));
}

String String::operator+(const String &other) const {
  String result(*this);
  result.append(other);
  return result;
}

bool String::operator==(const String &other) const { return data == other.data; }

bool String::operator!=(const String &other) const { return !(*this == other); }

bool String::operator<(const String &other) const {
  return CharIterator::compare(Slice(data.data(), data.size()),
                               Slice(other.data.data(), other.data.size())) < 0;
}

bool String::operator<=(const String &other) const {
  return CharIterator::compare(Slice(data.data(), data.size()),
                               Slice(other.data.data(), other.data.size())) <= 0;
}

bool String::operator>(const String &other) const { return !(*this <= other); }

bool String::operator>=(const String &other) const { return !(*this < other); }

} // namespace amelia
