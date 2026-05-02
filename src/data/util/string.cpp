#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>

#include "prelude.h"
#include "string.h"

namespace amelia {

String::String() noexcept { m_str.push_back('\0'); };

String::String(Slice<const char> str) {
  if (str.size() > 0) {
    CharIterator::validate(str);
    m_str.assign(str.ptr(), str.ptr() + str.size());
  }
  m_str.push_back('\0');
}

String::String(Text text) {
  m_str.assign(text.data().ptr(), text.data().ptr() + text.data().size());
  m_str.push_back('\0');
}

const char *String::c_str() const noexcept { return m_str.data(); }

Slice<const char> String::data() const noexcept { return Slice(m_str.data(), m_str.size() - 1); }

size_t String::size() const noexcept { return m_str.size() - 1; }

void String::append(Slice<const char> str) {
  if (str.size() > 0) {
    CharIterator::validate(str);
    m_str.pop_back();
    m_str.insert(m_str.end(), str.ptr(), str.ptr() + str.size());
    m_str.push_back('\0');
  }
}

void String::append(Text other) {
  if (other.size() > 0) {
    m_str.pop_back();
    m_str.insert(m_str.end(), other.data().ptr(), other.data().ptr() + other.data().size());
    m_str.push_back('\0');
  }
}

void String::append(uint32_t code_point) {
  m_str.pop_back();
  CharIterator::append(m_str, code_point);
  m_str.push_back('\0');
}

void String::assign(Text text) {
  m_str.assign(text.data().ptr(), text.data().ptr() + text.data().size());
  m_str.push_back('\0');
}

void String::clear() noexcept {
  m_str.clear();
  m_str.push_back('\0');
}

Text String::text() const noexcept {
  Text result;
  result.m_slice = Slice(m_str.data(), m_str.size() - 1);
  return result;
}

CharIterator String::begin() const { return CharIterator(data()); }

CharIterator String::end() const {
  return CharIterator(Slice<const char>(m_str.data() + m_str.size() - 1, 0));
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
  result.m_str.assign(str.c_str(), str.c_str() + str.size());
  result.m_str.push_back('\0');
  return result;
}

String String::from(std::vector<char> str) {
  CharIterator::validate(Slice(static_cast<const char *>(str.data()), str.size()));
  str.push_back('\0');
  String result;
  result.m_str = std::move(str);
  return result;
}

} // namespace amelia

namespace std {
size_t hash<amelia::String>::operator()(const amelia::String &obj) const {
  return std::hash<std::string_view>{}(std::string_view(obj.data().ptr(), obj.size()));
}
} // namespace std
