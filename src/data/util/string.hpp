#pragma once

#include <cstddef>

#include "data/util/abstract_string.hpp"
#include "data/util/list.hpp"
#include "data/util/text.hpp"

namespace amelia {

template <typename T> class ConstSlice;
class CharIterator;

/**
 * @class String
 * @brief A simple string class. Guaranteed to contain valid UTF-8 data.
 */
class String : public AbstractString {
public:
  /**
   * @brief Construct an empty String.
   */
  String() noexcept;

  /**
   * @brief Construct a String from a string literal or sequence of bytes. Must be valid UTF-8.
   */
  template <size_t N> String(const char (&str)[N]) : String(Text(str)) {}

  /**
   * @brief Construct a String from a sequence of bytes. Must be valid UTF-8.
   */
  explicit String(ConstSlice<char> str);

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
  ConstSlice<char> data() const noexcept;

  /**
   * @return A Text object representing the contents of this String. The Text object is valid until
   * the next non-const method call on this String instance.
   */
  Text text() const noexcept override;

  /**
   * @return The size of the string in bytes, not including the null terminator.
   */
  size_t size() const noexcept;

  /**
   * @brief Appends a string to this String. The input string must be valid UTF-8.
   * @param str A null-terminated C-style string to append.
   * @throws InvalidUTF8Error if the input string is not valid UTF-8.
   */
  void append(ConstSlice<char> str);

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
   * @brief Clears the contents of this String, making it empty.
   */
  void clear() noexcept;

  /**
   * @brief Compute a hash code for this String.
   */
  uint64_t hash_code() const noexcept;

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
   * @brief Concatenates this String with a Text object and returns the result as a new String.
   */
  String operator+(Text other) const;

  /**
   * @brief Appends another String to this String.
   */
  String &operator+=(const String &other);

  /**
   * @brief Appends a Text object to this String.
   */
  String &operator+=(Text other);

  /**
   * @brief Compares this String with another String for equality.
   */
  bool operator==(const String &other) const;

  /**
   * @brief Compares this String with another String for inequality.
   */
  bool operator!=(const String &other) const;

  /**
   * @brief Compares this String with another String for lexicographical order.
   */
  bool operator<(const String &other) const;

  /**
   * @brief Compares this String with another String for lexicographical order.
   */
  bool operator<=(const String &other) const;

  /**
   * @brief Compares this String with another String for lexicographical order.
   */
  bool operator>(const String &other) const;

  /**
   * @brief Compares this String with another String for lexicographical order.
   */
  bool operator>=(const String &other) const;

  /**
   * @brief Converts this String to a Text object. The returned Text shares the same underlying
   * data as this String, so it is valid until the next non-const method call on this String
   * instance.
   */
  operator Text() const noexcept;

  /**
   * @brief Constructs a String from a null-terminated C-style string. The input string must be
   * valid UTF-8.
   * @throws InvalidUTF8Error if the input string is not valid UTF-8.
   */
  static String from(const char *c_str);

  /**
   * @brief Constructs a String from a vector of chars. The input vector must contain valid UTF-8
   * data.
   * @throws InvalidUTF8Error if the input vector does not contain valid UTF-8 data.
   */
  static String from(List<char> str);

private:
  List<char> m_str;
};

} // namespace amelia
