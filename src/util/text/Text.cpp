#include "util/text/Text.h"
#include "util/text/CharIterator.h"
#include <cstring>

namespace amelia {

Text::Text(Slice<const char> str) : data(str) { CharIterator::validate(data); }

size_t Text::size() const noexcept { return data.size(); }

CharIterator Text::begin() const { return CharIterator(data); }

CharIterator Text::end() const { return CharIterator(data + size()); }

bool Text::operator==(const Text &other) const { return data == other.data; }

bool Text::operator!=(const Text &other) const { return !(*this == other); }

bool Text::operator<(const Text &other) const {
  return CharIterator::compare(data, other.data) < 0;
}

bool Text::operator<=(const Text &other) const {
  return CharIterator::compare(data, other.data) <= 0;
}

bool Text::operator>(const Text &other) const { return !(*this <= other); }

bool Text::operator>=(const Text &other) const { return !(*this < other); }

} // namespace amelia
