#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

#include "CharIterator.h"
#include "String.h"
#include "Text.h"

namespace amelia {

String::String() noexcept = default;

String::String(Slice<const char> str) {
  CharIterator::validate(str);
  if (str.size() > 0)
    data_str.assign(&*str.begin(), str.size());
}

String::String(Text text) : String(text.data()) {}

const char *String::c_str() const noexcept { return data_str.c_str(); }

Slice<const char> String::data() const noexcept { return Slice(data_str.data(), data_str.size()); }

size_t String::size() const noexcept { return data_str.size(); }

String &String::append(Slice<const char> str) {
  CharIterator::validate(str);
  if (str.size() > 0)
    data_str.append(&*str.begin(), str.size());
  return *this;
}

String &String::append(Text other) {
  data_str.append(other.data().ptr(), other.data().size());
  return *this;
}

String &String::append(uint32_t code_point) {
  CharIterator::append(code_point, data_str);
  return *this;
}

CharIterator String::begin() const {
  return CharIterator(Slice<const char>(data_str.data(), data_str.size()));
}

CharIterator String::end() const {
  return CharIterator(Slice<const char>(data_str.data() + data_str.size(), 0));
}

String String::operator+(const String &other) const {
  String result(*this);
  result.append(other);
  return result;
}

bool String::operator==(const String &other) const { return data_str == other.data_str; }

bool String::operator!=(const String &other) const { return !(*this == other); }

bool String::operator<(const String &other) const {
  return CharIterator::compare(Slice(data_str.data(), data_str.size()),
                               Slice(other.data_str.data(), other.data_str.size())) < 0;
}

bool String::operator<=(const String &other) const {
  return CharIterator::compare(Slice(data_str.data(), data_str.size()),
                               Slice(other.data_str.data(), other.data_str.size())) <= 0;
}

bool String::operator>(const String &other) const { return !(*this <= other); }

bool String::operator>=(const String &other) const { return !(*this < other); }

} // namespace amelia
