#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace amelia {

class String;

/**
 * @class CharIterator
 * @brief An iterator over the unicode code points in a String.
 */
class CharIterator {
public:
  CharIterator(const char *str);

  /**
   * @return The next Unicode code point in the string and advances the iterator.
   * @throws InvalidUTF8Error if the iterator is not currently pointing to a valid UTF-8 sequence.
   */
  uint32_t operator++();

  /**
   * @return The current Unicode code point in the string without advancing the iterator.
   * @throws InvalidUTF8Error if the iterator is not currently pointing to a valid UTF-8 sequence.
   */
  uint32_t operator*() const;

  /**
   * @brief Compares this iterator with another for equality. Two iterators are equal if they point
   * to the same position in the same string.
   */
  bool operator==(const CharIterator &other) const;

  /**
   * @brief Compares this iterator with another for inequality. Two iterators are not equal if
   * they do not point to the same position in the same string.
   */
  bool operator!=(const CharIterator &other) const;

  /**
   * @brief Validates that the given C-style string is valid UTF-8.
   * @throws InvalidUTF8Error if the input string is not valid UTF-8, from beginning to end.
   */
  static void validate(const char *str);

  /**
   * @brief Appends a single UTF32 code point value to a std::string as UTF-8.
   * @throws InvalidUTF8Error if the value is not a valid UTF32 code point, or if the
   * resulting str is not valid UTF-8.
   */
  static void append(uint32_t code_point, std::string &str);
private:
  const char *current;
};

} // namespace amelia
