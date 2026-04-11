#include "util/text/Text.h"
#include "util/text/CharIterator.h"
#include <cstring>

#include "String.h"
#include "Text.h"

namespace amelia {

Text::Text(Slice<const char> str) : data_slice(str) { CharIterator::validate(data_slice); }

Text::Text(const String &str) : data_slice(str.data()) {}

Slice<const char> Text::data() const noexcept { return data_slice; }

size_t Text::size() const noexcept { return data_slice.size(); }

CharIterator Text::begin() const { return CharIterator(data_slice); }

CharIterator Text::end() const { return CharIterator(data_slice + size()); }

bool Text::operator==(const Text &other) const { return data_slice == other.data_slice; }

bool Text::operator!=(const Text &other) const { return !(*this == other); }

bool Text::operator<(const Text &other) const {
  return CharIterator::compare(data_slice, other.data_slice) < 0;
}

bool Text::operator<=(const Text &other) const {
  return CharIterator::compare(data_slice, other.data_slice) <= 0;
}

bool Text::operator>(const Text &other) const { return !(*this <= other); }

bool Text::operator>=(const Text &other) const { return !(*this < other); }

} // namespace amelia
