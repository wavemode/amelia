#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "util/slice/Slice.h"

namespace amelia {

/**
 * @class CharIterator
 * @brief An iterator over the unicode code points in a UTF-8 string.
 */
class CharIterator {
public:
  /**
   * @brief Constructs an iterator over the UTF-8 code points in the given span.
   */
  explicit CharIterator(Slice<const char> str) noexcept;

  /**
   * @return Advances the iterator.
   * @throws InvalidUTF8Error if the iterator is not currently pointing to a valid UTF-8 sequence.
   */
  CharIterator &operator++();
  CharIterator operator++(int);

  /**
   * @return The current UTF-8 code point in the string without advancing the iterator.
   * @throws InvalidUTF8Error if the iterator is not currently pointing to a valid UTF-8 sequence.
   */
  uint32_t operator*() const;

  /**
   * @brief Compares this iterator with another for equality. Two iterators are equal if they point
   * to the same position in the same string.
   */
  bool operator==(const CharIterator &other) const noexcept;

  /**
   * @brief Compares this iterator with another for inequality. Two iterators are not equal if
   * they do not point to the same position in the same string.
   */
  bool operator!=(const CharIterator &other) const noexcept;

  /**
   * @brief Checks if the iterator has reached the end of the span.
   */
  bool at_end() const noexcept;

  /**
   * @brief Validates that the given span of bytes is valid UTF-8.
   * @throws InvalidUTF8Error if the input span is not valid UTF-8 from beginning to end.
   */
  static void validate(Slice<const char> str);

  /**
   * @brief Appends a single Unicode code point value to a std::string as UTF-8.
   * @throws InvalidUTF8Error if the value is not a valid Unicode code point.
   */
  static void append(uint32_t code_point, std::string &str);

private:
  Slice<const char>::Iterator current;
  Slice<const char>::Iterator end;
};

} // namespace amelia
