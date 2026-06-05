#include <cstring>

#include "prelude.hpp"
#include "text.hpp"

namespace amelia {

Text::Text() noexcept = default;

Text::Text(ConstSlice<char> str) : m_slice(str) {
  CharIterator::validate(m_slice);
}

ConstSlice<char> Text::data() const noexcept {
  return m_slice;
}

size_t Text::size() const noexcept {
  return m_slice.size();
}

CharIterator Text::begin() const noexcept {
  return CharIterator(m_slice);
}

CharIterator Text::end() const noexcept {
  return CharIterator(m_slice.end());
}

bool Text::operator==(const Text &other) const noexcept {
  return CharIterator::compare(m_slice, other.m_slice) == 0;
}

bool Text::operator!=(const Text &other) const noexcept {
  return !(*this == other);
}

bool Text::operator<(const Text &other) const noexcept {
  return CharIterator::compare(m_slice, other.m_slice) < 0;
}

bool Text::operator<=(const Text &other) const noexcept {
  return CharIterator::compare(m_slice, other.m_slice) <= 0;
}

bool Text::operator>(const Text &other) const noexcept {
  return !(*this <= other);
}

bool Text::operator>=(const Text &other) const noexcept {
  return !(*this < other);
}

Text Text::from(const char *c_str) {
  return Text(ConstSlice(c_str, std::strlen(c_str)));
}

uint64_t Text::hash_code() const noexcept {
  return amelia::hash_str_64(m_slice.ptr(), m_slice.size());
}

} // namespace amelia
