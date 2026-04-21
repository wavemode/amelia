#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "interface/text/IString.h"
#include "util/slice/Slice.h"
#include "util/text/CharIterator.h"
#include "util/text/Text.h"

namespace amelia {

/**
 * @class String
 * @brief A simple string class. Guaranteed to contain valid UTF-8 data.
 */
class String : public IString {
public:
  /**
   * @brief Construct an empty String.
   */
  String() noexcept;

  /**
   * @brief Construct a String from a string literal or sequence of bytes. Must be valid UTF-8.
   */
  template <size_t N> String(const char (&str)[N]) {
    if (N == 0) {
      data_str = "";
      return;
    }

    if (str[N - 1] == '\0') {
      *this = String(Slice(str, N - 1));
    } else {
      *this = String(Slice(str, N));
    }
  }

  /**
   * @brief Construct a String from a sequence of bytes. Must be valid UTF-8.
   */
  explicit String(Slice<const char> str);

  /**
   * @brief Construct a String from a Text object.
   */
  explicit String(Text text);

  /**
   * @return A pointer to a null-terminated C-style string. The pointer is valid
   * until the next non-const method call on this String instance.
   */
  const char *c_str() const noexcept override;

  /**
   * @return A Slice representing the underlying UTF-8 data of this String.
   * The slice is valid until the next non-const method call on this String instance.
   */
  Slice<const char> data() const noexcept;

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
  void append(Slice<const char> str);

  /**
   * @brief Appends another String to this String.
   * @param other The text to append.
   */
  void append(Text other) override;

  /**
   * @brief Append a single Unicode code point to this String.
   * @param codePoint The Unicode code point to append.
   */
  void append(uint32_t code_point) override;

  /**
   * @brief Assigns new text to this String, replacing its current contents.
   */
  void assign(Text text) override;

  /**
   * @return A Text object representing the contents of this String.
   */
  Text text() const noexcept override;

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

  /**
   * @brief Converts this String to a Text object. The returned Text shares the same underlying
   * data as this String, so it is valid until the next non-const method call on this String
   * instance.
   */
  operator Text() const noexcept;

  /**
   * @brief Constructs a String from a std::string. The input string must be valid UTF-8.
   * @throws InvalidUTF8Error if the input string is not valid UTF-8.
   */
  static String from(std::string str);

private:
  std::string data_str;
};

} // namespace amelia
