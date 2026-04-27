#include "String.h"
#include "Prelude.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>

namespace amelia {

String::String() noexcept { data_str.push_back('\0'); };

String::String(Slice<const char> str) {
  if (str.size() > 0) {
    CharIterator::validate(str);
    data_str.assign(str.ptr(), str.ptr() + str.size());
  }
  data_str.push_back('\0');
}

String::String(Text text) {
  data_str.assign(text.data().ptr(), text.data().ptr() + text.data().size());
  data_str.push_back('\0');
}

const char *String::c_str() const noexcept { return data_str.data(); }

Slice<const char> String::data() const noexcept {
  return Slice(data_str.data(), data_str.size() - 1);
}

size_t String::size() const noexcept { return data_str.size() - 1; }

void String::append(Slice<const char> str) {
  if (str.size() > 0) {
    CharIterator::validate(str);
    data_str.pop_back();
    data_str.insert(data_str.end(), str.ptr(), str.ptr() + str.size());
    data_str.push_back('\0');
  }
}

void String::append(Text other) {
  if (other.size() > 0) {
    data_str.pop_back();
    data_str.insert(data_str.end(), other.data().ptr(), other.data().ptr() + other.data().size());
    data_str.push_back('\0');
  }
}

void String::append(uint32_t code_point) {
  data_str.pop_back();
  CharIterator::append(code_point, data_str);
  data_str.push_back('\0');
}

void String::assign(Text text) {
  data_str.assign(text.data().ptr(), text.data().ptr() + text.data().size());
  data_str.push_back('\0');
}

void String::clear() noexcept {
  data_str.clear();
  data_str.push_back('\0');
}

Text String::text() const noexcept {
  Text result;
  result.data_slice = Slice(data_str.data(), data_str.size() - 1);
  return result;
}

CharIterator String::begin() const { return CharIterator(data()); }

CharIterator String::end() const {
  return CharIterator(Slice<const char>(data_str.data() + data_str.size() - 1, 0));
}

String String::operator+(const String &other) const {
  String result(*this);
  result.append(other);
  return result;
}

String &String::operator+=(const String &other) {
  append(other);
  return *this;
}

String String::operator+(Text other) const {
  String result(*this);
  result.append(other);
  return result;
}

String &String::operator+=(Text other) {
  append(other);
  return *this;
}

bool String::operator==(const String &other) const {
  return size() == other.size() && std::memcmp(c_str(), other.c_str(), size()) == 0;
}

bool String::operator!=(const String &other) const { return !(*this == other); }

bool String::operator<(const String &other) const {
  return CharIterator::compare(data(), other.data()) < 0;
}

bool String::operator<=(const String &other) const {
  return CharIterator::compare(data(), other.data()) <= 0;
}

bool String::operator>(const String &other) const { return !(*this <= other); }

bool String::operator>=(const String &other) const { return !(*this < other); }

String::operator Text() const noexcept { return text(); }

String String::from(const std::string &str) {
  CharIterator::validate(Slice(str.c_str(), str.size()));
  String result;
  result.data_str.assign(str.c_str(), str.c_str() + str.size());
  result.data_str.push_back('\0');
  return result;
}

} // namespace amelia

namespace std {
size_t hash<amelia::String>::operator()(const amelia::String &obj) const {
  return std::hash<std::string_view>{}(std::string_view(obj.data().ptr(), obj.size()));
}
} // namespace std
