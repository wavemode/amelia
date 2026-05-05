#include <cstring>
#include <string_view>

#include "prelude.h"
#include "text.h"

namespace amelia {

Text::Text() noexcept = default;

Text::Text(ConstSlice<char> str) : m_slice(str) { CharIterator::validate(m_slice); }

Text::Text(const std::string &str) : Text(ConstSlice(str.c_str(), str.size())) {}

ConstSlice<char> Text::data() const noexcept { return m_slice; }

size_t Text::size() const noexcept { return m_slice.size(); }

CharIterator Text::begin() const noexcept { return CharIterator(m_slice); }

CharIterator Text::end() const noexcept { return CharIterator(m_slice.end()); }

bool Text::operator==(const Text &other) const noexcept {
  return m_slice.size() == other.m_slice.size() &&
         std::memcmp(m_slice.ptr(), other.m_slice.ptr(), m_slice.size()) == 0;
}

bool Text::operator!=(const Text &other) const noexcept { return !(*this == other); }

bool Text::operator<(const Text &other) const noexcept {
  return CharIterator::compare(m_slice, other.m_slice) < 0;
}

bool Text::operator<=(const Text &other) const noexcept {
  return CharIterator::compare(m_slice, other.m_slice) <= 0;
}

bool Text::operator>(const Text &other) const noexcept { return !(*this <= other); }

bool Text::operator>=(const Text &other) const noexcept { return !(*this < other); }

Text Text::from(const std::string &str) { return Text(ConstSlice(str.c_str(), str.size())); }

Text Text::from(const char *c_str) { return Text(ConstSlice(c_str, std::strlen(c_str))); }

} // namespace amelia

namespace std {

size_t hash<amelia::Text>::operator()(const amelia::Text &obj) const {
  return std::hash<std::string_view>{}(std::string_view(obj.data().ptr(), obj.size()));
}

} // namespace std
