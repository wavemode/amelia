#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "data/util/abstract_iterator.h"
#include "data/util/slice.h"

namespace amelia {

template <typename T> class Slice;
template <typename T> class List;
class Text;

/**
 * @class CharIterator
 * @brief An iterator over the unicode code points in a UTF-8 string.
 */
class CharIterator : public AbstractIterator<uint32_t> {
public:
  /**
   * @brief Constructs an iterator over the UTF-8 code points in the given span.
   */
  explicit CharIterator(ConstSliceIterator<char> str) noexcept;

  /**
   * @brief Constructs an iterator over the UTF-8 code points in the given Text object.
   */
  explicit CharIterator(Text text) noexcept;

  /**
   * @return Advances the iterator.
   * @throws InvalidUTF8Error if the iterator is not currently pointing to a valid UTF-8 sequence.
   * @throws std::out_of_range if the iterator is advanced past the end of the string.
   */
  CharIterator &operator++();

  /**
   * @return Advances the iterator.
   * @throws InvalidUTF8Error if the iterator is not currently pointing to a valid UTF-8 sequence.
   * @throws std::out_of_range if the iterator is advanced past the end of the string.
   */
  CharIterator operator++(int);

  /**
   * @return A copy of the iterator pointing to the current code point.
   */
  CharIterator begin() const noexcept;

  /**
   * @return An iterator pointing one past the end of the string.
   */
  CharIterator end() const noexcept;

  /**
   * @return The current UTF-8 code point in the string without advancing the iterator.
   * @throws InvalidUTF8Error if the iterator is not currently pointing to a valid UTF-8 sequence.
   * @throws std::out_of_range if the iterator is at the end of the string.
   */
  uint32_t operator*() const;

  /**
   * @brief Returns the next UTF-8 code point in the string without advancing the iterator.
   * @throws InvalidUTF8Error if the iterator is not currently pointing to a valid UTF-8 sequence.
   * @throws std::out_of_range if the iterator is at the end of the string.
   */
  uint32_t peek() override;

  /**
   * @brief Returns the next UTF-8 code point in the string without advancing the iterator.
   * @throws InvalidUTF8Error if the iterator is not currently pointing to a valid UTF-8 sequence.
   * @throws std::out_of_range if the iterator is at the end of the string.
   */
  uint32_t peek() const;

  /**
   * @brief Returns the next UTF-8 code point in the string and advances the iterator.
   * @throws InvalidUTF8Error if the iterator is not currently pointing to a valid UTF-8 sequence.
   * @throws std::out_of_range if the iterator is advanced past the end of the string.
   */
  uint32_t next() override;

  /**
   * @return The underlying slice of bytes representing the remaining span of the string.
   */
  ConstSlice<char> data() const noexcept;

  /**
   * @return A Text object representing the remaining span of the string.
   */
  Text text() const noexcept;

  /**
   * @return A new iterator advanced by n code points.
   * @throws InvalidUTF8Error if the iterator would encounter invalid UTF-8 data before advancing n
   * code points.
   * @throws std::out_of_range if the iterator would advance past the end of the string.
   */
  CharIterator plus(size_t n) const;

  /**
   * @return A new iterator advanced by at least n bytes. (That is, iterator will be advanced by the
   * smallest number of code points such that at least n bytes have been advanced.)
   * @throws InvalidUTF8Error if the iterator would encounter invalid UTF-8 data before advancing n
   * bytes.
   * @throws std::out_of_range if the iterator would advance past the end of the string.
   */
  CharIterator plus_bytes(size_t n) const;

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
  bool at_end() const noexcept override;

  /**
   * @brief Validates that the given span of bytes is valid UTF-8.
   * @throws InvalidUTF8Error if the input span is not valid UTF-8 from beginning to end.
   */
  static void validate(ConstSlice<char> str);

  /**
   * @brief Appends a single Unicode code point value to a std::string as UTF-8.
   * @throws InvalidUTF8Error if the value is not a valid Unicode code point.
   */
  static void append(List<char> &str, uint32_t code_point);

  /**
   * @brief Compares two UTF-8 encoded slices lexicographically by Unicode code points.
   * @return A signed char indicating the result of the comparison: negative if a < b, zero if a ==
   * b, positive if a > b.
   */
  static signed char compare(ConstSlice<char> a, ConstSlice<char> b);

private:
  ConstSliceIterator<char> m_slice_iter;
};

} // namespace amelia
