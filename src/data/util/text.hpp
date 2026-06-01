#pragma once

#include <cstddef>
#include <string>

#include "data/util/slice.hpp"

namespace amelia {

class CharIterator;
class String;
struct TextUtils;

/**
 * @class Text
 * @brief A reference to a sequence of UTF-8 data. Basically, a rich wrapper over ConstSlice<char>.
 */
class Text {
public:
  /**
   * Construct an empty Text instance.
   */
  Text() noexcept;

  /**
   * @brief Construct a Text from a string literal or sequence of bytes. Must be valid UTF-8. Can
   * optionally be null-terminated.
   * @throws InvalidUTF8Error if the input slice is not valid UTF-8 from beginning to end.
   */
  template <size_t N>
  Text(const char (&str)[N]) : Text(ConstSlice(str, (N > 0 && str[N - 1] == '\0') ? N - 1 : N)) {}

  /**
   * @brief Construct a Text from a sequence of bytes. Must be valid UTF-8.
   * @throws InvalidUTF8Error if the input slice is not valid UTF-8 from beginning to end.
   */
  explicit Text(ConstSlice<char> str);

  /**
   * @brief Construct a Text from a std::string object.
   * @throws InvalidUTF8Error if the input string is not valid UTF-8 from beginning to end.
   */
  explicit Text(const std::string &str);

  /**
   * @return A Slice representing the underlying UTF-8 data of this Text.
   * The slice is valid until the next non-const method call on this Text instance.
   */
  ConstSlice<char> data() const noexcept;

  /**
   * @return The size of the Text in bytes, not including the null
   * terminator.
   */
  size_t size() const noexcept;

  /**
   * @return An iterator pointing to the first Unicode code point in the Text.
   */
  CharIterator begin() const noexcept;

  /**
   * @return An iterator pointing one past the end of the Text.
   */
  CharIterator end() const noexcept;

  /**
   * @brief Compares this Text with another Text for equality.
   */
  bool operator==(const Text &other) const noexcept;

  /**
   * @brief Compares this Text with another Text for inequality.
   */
  bool operator!=(const Text &other) const noexcept;

  /**
   * @brief Compares this Text with another Text for lexicographical order. The comparison is
   * based on the sequence of Unicode code points.
   */
  bool operator<(const Text &other) const noexcept;

  /**
   * @brief Compares this Text with another Text for lexicographical order. The comparison is
   * based on the sequence of Unicode code points.
   */
  bool operator<=(const Text &other) const noexcept;

  /**
   * @brief Compares this Text with another Text for lexicographical order. The comparison is
   * based on the sequence of Unicode code points.
   */
  bool operator>(const Text &other) const noexcept;

  /**
   * @brief Compares this Text with another Text for lexicographical order. The comparison is
   * based on the sequence of Unicode code points.
   */
  bool operator>=(const Text &other) const noexcept;

  /**
   * @brief Creates a Text from a std::string. The input string must be valid UTF-8.
   * @throws InvalidUTF8Error if the input string is not valid UTF-8 from beginning to end.
   */
  static Text from(const std::string &str);

  /**
   * @brief Creates a Text from a null-terminated C-style string. The input string must be valid
   * UTF-8.
   * @throws InvalidUTF8Error if the input string is not valid UTF-8 from beginning to end.
   */
  static Text from(const char *c_str);

  friend struct TextUtils;
  friend class String;

private:
  ConstSlice<char> m_slice;
};

} // namespace amelia

namespace std {
template <> struct hash<amelia::Text> {
  size_t operator()(const amelia::Text &obj) const;
};
} // namespace std
