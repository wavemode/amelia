#include "Text.h"
#include "Prelude.h"

#include <cstring>

namespace amelia {

Text::Text() noexcept = default;

Text::Text(Slice<const char> str) : data_slice(str) { CharIterator::validate(data_slice); }

Text::Text(const std::string &str) : Text(Slice(str.c_str(), str.size())) {}

Slice<const char> Text::data() const noexcept { return data_slice; }

size_t Text::size() const noexcept { return data_slice.size(); }

CharIterator Text::begin() const noexcept { return CharIterator(data_slice); }

CharIterator Text::end() const noexcept { return CharIterator(data_slice.end()); }

bool Text::operator==(const Text &other) const noexcept { return data_slice == other.data_slice; }

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
