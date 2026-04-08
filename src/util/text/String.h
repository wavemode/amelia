#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

namespace amelia {

class CharIterator;

/**
 * @class String
 * @brief A simple string class. Guaranteed to contain valid UTF-8 data.
 */
class String {
public:
  /**
   * @brief Constructs an empty String.
   */
  String() noexcept;

  /**
   * @brief Constructs a String from a string literal or sequence of bytes. Must be valid UTF-8.
   */
  template <size_t N> String(const char (&str)[N]) : String(static_cast<const char *>(str)) {
    CharIterator::validate(str, str + N);
  }

  /**
   * @return A pointer to a null-terminated C-style string. The pointer is valid
   * until the next non-const method call on this String instance.
   */
  const char *c_str() const noexcept;

  /**
   * @return The length of the string in bytes, not including the null
   * terminator.
   */
  size_t size() const noexcept;

  /**
   * @brief Appends a string to this String. The input string must be valid UTF-8.
   * @param str A null-terminated C-style string to append.
   * @throws InvalidUTF8Error if the input string is not valid UTF-8.
   */
  void append(std::span<const char> str);

  /**
   * @brief Appends another String to this String.
   * @param other The String to append.
   */
  void append(const String &other);

  /**
   * @brief Append a single Unicode code point to this String.
   * @param codePoint The Unicode code point to append.
   */
  void append(uint32_t code_point);

  /**
   * @return An iterator pointing to the first Unicode code point in the string.
   */
  CharIterator begin() const;

  /**
   * @return An iterator pointing to the null terminator at the end of the String.
   */
  CharIterator end() const;

  /**
   * @brief Concatenates this String with another String and returns the result as a new String.
   */
  String operator+(const String &other) const;

  /**
   * @brief Compares this String with another String for equality. Two Strings are equal if they
   * contain the same sequence of Unicode code points.
   */
  bool operator==(const String &other) const;

  /**
   * @brief Compares this String with another String for inequality. Two Strings are not equal if
   * they do not contain the same sequence of Unicode code points.
   */
  bool operator!=(const String &other) const;

  /**
   * @brief Compares this String with another String for lexicographical order. The comparison is
   * based on the sequence of Unicode code points in the strings.
   */
  bool operator<(const String &other) const;

  /**
   * @brief Compares this String with another String for lexicographical order. The comparison is
   * based on the sequence of Unicode code points in the strings.
   */
  bool operator<=(const String &other) const;

  /**
   * @brief Compares this String with another String for lexicographical order. The comparison is
   * based on the sequence of Unicode code points in the strings.
   */
  bool operator>(const String &other) const;

  /**
   * @brief Compares this String with another String for lexicographical order. The comparison is
   * based on the sequence of Unicode code points in the strings.
   */
  bool operator>=(const String &other) const;

private:
  /**
   * @brief Constructs a String from a null-terminated C-style string. Must be valid UTF-8.
   * @param str A null-terminated C-style string.
   * @throws InvalidUTF8Error if the input string is not valid UTF-8.
   */
  String(const char *str);

  std::string data;
};

} // namespace amelia
