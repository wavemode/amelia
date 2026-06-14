#include <cstdint>
#include <cstring>

#include "prelude.hpp"
#include "string.hpp"

namespace amelia {

String::String() noexcept {
  m_str.push_back('\0');
};

String::String(ConstSlice<char> str) {
  if (str.size() > 0) {
    CharIterator::validate(str);
    m_str.assign(str);
  }
  m_str.push_back('\0');
}

String::String(Text text) {
  m_str.assign(text.data());
  m_str.push_back('\0');
}

const char *String::c_str() const noexcept {
  return m_str.data().ptr();
}

Slice<char> String::data() noexcept {
  return Slice(m_str.data().ptr(), m_str.size() - 1);
}

ConstSlice<char> String::data() const noexcept {
  return ConstSlice(m_str.data().ptr(), m_str.size() - 1);
}

size_t String::size() const noexcept {
  return m_str.size() - 1;
}

void String::reserve(size_t new_capacity) {
  if ((new_capacity + 1) > m_str.capacity()) {
    m_str.reserve(new_capacity + 1);
  }
}

void String::append(ConstSlice<char> str) {
  if (str.size() > 0) {
    CharIterator::validate(str);
    m_str.pop_back();
    m_str.append(str);
    m_str.push_back('\0');
  }
}

void String::append(Text other) {
  if (other.size() > 0) {
    m_str.pop_back();
    m_str.append(other.data());
    m_str.push_back('\0');
  }
}

void String::append(uint32_t code_point) {
  m_str.pop_back();
  CharIterator::append(m_str, code_point);
  m_str.push_back('\0');
}

void String::assign(Text text) {
  m_str.assign(text.data());
  m_str.push_back('\0');
}

void String::clear() noexcept {
  m_str.clear();
  m_str.push_back('\0');
}

Text String::text() const noexcept {
  Text result;
  result.m_slice = ConstSlice(m_str.data().ptr(), m_str.size() - 1);
  return result;
}

uint64_t String::hash_code() const noexcept {
  return amelia::hash(text());
}

CharIterator String::begin() const {
  return CharIterator(data());
}

CharIterator String::end() const {
  return CharIterator(data().end());
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
  return CharIterator::compare(data(), other.data()) == 0;
}

bool String::operator!=(const String &other) const {
  return !(*this == other);
}

bool String::operator<(const String &other) const {
  return CharIterator::compare(data(), other.data()) < 0;
}

bool String::operator<=(const String &other) const {
  return CharIterator::compare(data(), other.data()) <= 0;
}

bool String::operator>(const String &other) const {
  return !(*this <= other);
}

bool String::operator>=(const String &other) const {
  return !(*this < other);
}

String::operator Text() const noexcept {
  return text();
}

String String::from(const char *c_str) {
  return String(Text::from(c_str));
}

String String::from_owned(char *c_str) {
  Text text = Text::from(c_str);
  String result;
  result.m_str = List<char>::from_owned(c_str, text.size());
  return result;
}

String String::from(List<char> str) {
  CharIterator::validate(str.data());
  str.push_back('\0');
  String result;
  result.m_str = move(str);
  return result;
}

} // namespace amelia
