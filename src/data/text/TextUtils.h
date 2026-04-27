#pragma once

#include <cstddef>
#include <cstdint>
#include <variant>

#include "data/text/Text.h"

namespace amelia {

template <typename T> class IList;
template <typename T> class Slice;
class String;
class IString;
class CharIterator;

struct TextUtils {
  static Text WHITESPACE_CHARS;

  static void split(IList<Text> &output, Text input, Text delimiter, int64_t max_splits = -1);
  static void split(IList<String> &output, Text input, Text delimiter, int64_t max_splits = -1);

  static void join_into(IString &output, Slice<Text> parts, Text delimiter);

  static Text trim(Text input, Text chars = WHITESPACE_CHARS);
  static void trim_into(IString &output, Text input, Text chars = WHITESPACE_CHARS);

  static Text trim_left(Text input, Text chars = WHITESPACE_CHARS);
  static void trim_left_into(IString &output, Text input, Text chars = WHITESPACE_CHARS);

  static Text trim_right(Text input, Text chars = WHITESPACE_CHARS);
  static void trim_right_into(IString &output, Text input, Text chars = WHITESPACE_CHARS);

  static void to_lower(IString &text);
  static void to_lower_into(IString &output, Text input);

  static void to_upper(IString &text);
  static void to_upper_into(IString &output, Text input);

  static bool contains(Text input, Text substring);
  static bool starts_with(Text input, Text prefix);
  static bool ends_with(Text input, Text suffix);

  static CharIterator find(Text input, Text substring);
  static int64_t find_byte(Text input, Text substring);

  static CharIterator find_after(Text input, Text substring, CharIterator start);
  static int64_t find_after_byte(Text input, Text substring, size_t index_start);

  static CharIterator find_before(Text input, Text substring, CharIterator end);
  static int64_t find_before_byte(Text input, Text substring, size_t index_end);

  static CharIterator find_char(Text input, uint32_t code_point);
  static int64_t find_char_byte(Text input, uint32_t code_point);

  static CharIterator find_char_after(Text input, uint32_t code_point, CharIterator start);
  static int64_t find_char_after_byte(Text input, uint32_t code_point, size_t index_start);

  static CharIterator find_char_before(Text input, uint32_t code_point, CharIterator end);
  static int64_t find_char_before_byte(Text input, uint32_t code_point, size_t index_end);

  static void find_all(IList<CharIterator> &output, Text input, Text substring);
  static void find_all_bytes(IList<int64_t> &output, Text input, Text substring);

  static void find_all_chars(IList<CharIterator> &output, Text input, uint32_t code_point);
  static void find_all_char_bytes(IList<int64_t> &output, Text input, uint32_t code_point);

  static void replace(IString &text, Text search, Text replacement, int64_t count = 1);
  static void replace_into(
      IString &output, Text input, Text search, Text replacement, int64_t count = 1
  );

  static void replace_all(IString &text, Text search, Text replacement);
  static void replace_all_into(IString &output, Text input, Text search, Text replacement);

  static void pad_left(IString &text, size_t total_length, Text pad = " ");
  static void pad_left_into(IString &output, Text input, size_t total_length, Text pad = " ");

  static void pad_right(IString &text, size_t total_length, Text pad = " ");
  static void pad_right_into(IString &output, Text input, size_t total_length, Text pad = " ");

  static void remove(IString &text, Text substring, int64_t count = 1);
  static void remove_into(IString &output, Text input, Text substring, int64_t count = 1);

  static void remove_all(IString &text, Text substring);
  static void remove_all_into(IString &output, Text input, Text substring);

  static void repeat(IString &text, size_t count);
  static void repeat_into(IString &output, Text input, size_t count);

  static void reverse(IString &text);
  static void reverse_into(IString &output, Text input);

  static Text strip_prefix(Text input, Text prefix);
  static Text strip_suffix(Text input, Text suffix);
  static Text slice(Text input, CharIterator char_start, size_t char_length);
  static Text slice_bytes(Text input, size_t index_start, size_t byte_length);
  static Text substr(Text input, CharIterator char_start, CharIterator char_end);
  static Text substr_bytes(Text input, size_t index_start, size_t index_end);
  static Text head(Text input, CharIterator char_end);
  static Text head_bytes(Text input, size_t index_end);
  static Text tail(Text input, CharIterator char_start);
  static Text tail_bytes(Text input, size_t index_start);

  static void to_string(IString &output, int64_t value);
  static void to_string(IString &output, size_t value);
  static void to_string(IString &output, double value);

  static std::variant<int64_t, double> parse_number(CharIterator it);

  static bool is_digit(Text input);
  static bool is_digit(uint32_t ch);

  static bool is_alpha(Text input);
  static bool is_alpha(uint32_t ch);

  static bool is_alnum(Text input);
  static bool is_alnum(uint32_t ch);

  static bool is_ascii(Text input);
  static bool is_ascii(uint32_t ch);

  static size_t count(Text input, Text substring);
  static size_t count_chars(Text input);

private:
  static Text text_from_subslice(Slice<const char> slice);
};

} // namespace amelia
