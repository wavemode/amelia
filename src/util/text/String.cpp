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

void String::append(Slice<const char> str) {
  if (str.size() > 0) {
    CharIterator::validate(str);
    data_str.append(&*str.begin(), str.size());
  }
}

void String::append(Text other) { data_str.append(other.data().ptr(), other.data().size()); }

void String::append(uint32_t code_point) { CharIterator::append(code_point, data_str); }

void String::assign(Text text) { data_str.assign(text.data().ptr(), text.data().size()); }

Text String::text() const noexcept { return Text(Slice(data_str.data(), data_str.size())); }

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
  return CharIterator::compare(
             Slice(data_str.data(), data_str.size()),
             Slice(other.data_str.data(), other.data_str.size())
         ) < 0;
}

bool String::operator<=(const String &other) const {
  return CharIterator::compare(
             Slice(data_str.data(), data_str.size()),
             Slice(other.data_str.data(), other.data_str.size())
         ) <= 0;
}

bool String::operator>(const String &other) const { return !(*this <= other); }

bool String::operator>=(const String &other) const { return !(*this < other); }

String::operator Text() const noexcept { return text(); }

String String::from(std::string str) {
  CharIterator::validate(Slice(str.c_str(), str.size()));
  String result;
  result.data_str = std::move(str);
  return result;
}

} // namespace amelia
