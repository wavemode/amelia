#pragma once

#include <cstddef>

#include "util/slice/Slice.h"

namespace amelia {

class CharIterator;
class String;

/**
 * @class Text
 * @brief A reference to a sequence of UTF-8 data. Basically, a rich wrapper over Slice<const char>.
 */
class Text {
public:
  /**
   * @brief Construct a Text from a string literal or sequence of bytes. Must be valid UTF-8. Can
   * optionally be null-terminated.
   * @throws InvalidUTF8Error if the input slice is not valid UTF-8 from beginning to end.
   */
  template <size_t N> Text(const char (&str)[N]) {
    if (N == 0) {
      data_slice = Slice(str, 0);
      return;
    }

    if (str[N - 1] == '\0') {
      *this = Text(Slice(str, N - 1));
    } else {
      *this = Text(Slice(str, N));
    }
  }

  /**
   * @brief Construct a Text from a sequence of bytes. Must be valid UTF-8.
   * @throws InvalidUTF8Error if the input slice is not valid UTF-8 from beginning to end.
   */
  explicit Text(Slice<const char> str);

  /**
   * @brief Construct a Text from a String.
   */
  Text(const String &str) noexcept;

  /**
   * @return A Slice representing the underlying UTF-8 data of this Text.
   * The slice is valid until the next non-const method call on this Text instance.
   */
  Slice<const char> data() const noexcept;

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
   * @brief Compares this Text with another Text for equality. Two Texts are equal if they
   * contain the same sequence of Unicode code points.
   */
  bool operator==(const Text &other) const noexcept;

  /**
   * @brief Compares this Text with another Text for inequality. Two Texts are not equal if
   * they do not contain the same sequence of Unicode code points.
   */
  bool operator!=(const Text &other) const noexcept;

  /**
   * @brief Compares this Text with another Text for lexicographical order. The comparison is
   * based on the sequence of Unicode code points in the Texts.
   */
  bool operator<(const Text &other) const noexcept;

  /**
   * @brief Compares this Text with another Text for lexicographical order. The comparison is
   * based on the sequence of Unicode code points in the Texts.
   */
  bool operator<=(const Text &other) const noexcept;

  /**
   * @brief Compares this Text with another Text for lexicographical order. The comparison is
   * based on the sequence of Unicode code points in the Texts.
   */
  bool operator>(const Text &other) const noexcept;

  /**
   * @brief Compares this Text with another Text for lexicographical order. The comparison is
   * based on the sequence of Unicode code points in the Texts.
   */
  bool operator>=(const Text &other) const noexcept;

private:
  Slice<const char> data_slice;
};

} // namespace amelia
