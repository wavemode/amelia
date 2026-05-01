#include <cstring>
#include <string_view>

#include "prelude.h"
#include "text.h"

namespace amelia {

Text::Text() noexcept = default;

Text::Text(Slice<const char> str) : data_slice(str) { CharIterator::validate(data_slice); }

Text::Text(const std::string &str) : Text(Slice(str.c_str(), str.size())) {}

Slice<const char> Text::data() const noexcept { return data_slice; }

size_t Text::size() const noexcept { return data_slice.size(); }

CharIterator Text::begin() const noexcept { return CharIterator(data_slice); }

CharIterator Text::end() const noexcept { return CharIterator(data_slice.end()); }

bool Text::operator==(const Text &other) const noexcept {
  return data_slice.size() == other.data_slice.size() &&
         std::memcmp(data_slice.ptr(), other.data_slice.ptr(), data_slice.size()) == 0;
}

bool Text::operator!=(const Text &other) const noexcept { return !(*this == other); }

bool Text::operator<(const Text &other) const noexcept {
  return CharIterator::compare(data_slice, other.data_slice) < 0;
}

bool Text::operator<=(const Text &other) const noexcept {
  return CharIterator::compare(data_slice, other.data_slice) <= 0;
}

bool Text::operator>(const Text &other) const noexcept { return !(*this <= other); }

bool Text::operator>=(const Text &other) const noexcept { return !(*this < other); }

Text Text::from(const std::string &str) { return Text(Slice(str.c_str(), str.size())); }

Text Text::from(const char *c_str) { return Text(Slice(c_str, std::strlen(c_str))); }

} // namespace amelia

namespace std {

size_t hash<amelia::Text>::operator()(const amelia::Text &obj) const {
  return std::hash<std::string_view>{}(std::string_view(obj.data().ptr(), obj.size()));
}

} // namespace std
