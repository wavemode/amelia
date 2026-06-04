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
  if (m_slice.size() != other.m_slice.size()) {
    return false;
  }

  if (m_slice.ptr() == other.m_slice.ptr()) {
    return true;
  }

  if (m_slice.ptr() == nullptr || other.m_slice.ptr() == nullptr) {
    // we already checked that both have the same size, so if one is null then both must be size 0
    return true;
  }

  return std::memcmp(m_slice.ptr(), other.m_slice.ptr(), m_slice.size()) == 0;
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
  uint64_t hash = 0xcbf29ce484222325;
  for (uint32_t cp : *this) {
    hash ^= amelia::hash(cp);
  }
  return hash;
}

} // namespace amelia
