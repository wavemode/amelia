#pragma once

#include <cstddef>
#include <cstdint>

namespace amelia {

template <typename T> struct AbstractList;
template <typename T> class Slice;
class String;
class Text;
struct AbstractString;
class CharIterator;

struct TextUtils {
  static Text WHITESPACE_CHARS;

  static void split(
      AbstractList<Text> &output, Text input, Text delimiter, int64_t max_splits = -1
  );
  static void split(
      AbstractList<String> &output, Text input, Text delimiter, int64_t max_splits = -1
  );

  static void join_into(AbstractString &output, ConstSlice<Text> parts, Text delimiter);

  static Text trim(Text input, Text chars = WHITESPACE_CHARS);
  static void trim_into(AbstractString &output, Text input, Text chars = WHITESPACE_CHARS);

  static Text trim_left(Text input, Text chars = WHITESPACE_CHARS);
  static void trim_left_into(AbstractString &output, Text input, Text chars = WHITESPACE_CHARS);

  static Text trim_right(Text input, Text chars = WHITESPACE_CHARS);
  static void trim_right_into(AbstractString &output, Text input, Text chars = WHITESPACE_CHARS);

  static void to_lower(AbstractString &text);
  static void to_lower_into(AbstractString &output, Text input);

  static void to_upper(AbstractString &text);
  static void to_upper_into(AbstractString &output, Text input);

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

  static void find_all(AbstractList<CharIterator> &output, Text input, Text substring);
  static void find_all_bytes(AbstractList<int64_t> &output, Text input, Text substring);

  static void find_all_chars(AbstractList<CharIterator> &output, Text input, uint32_t code_point);
  static void find_all_char_bytes(AbstractList<int64_t> &output, Text input, uint32_t code_point);

  static void replace(AbstractString &text, Text search, Text replacement, int64_t count = 1);
  static void replace_into(
      AbstractString &output, Text input, Text search, Text replacement, int64_t count = 1
  );

  static void replace_all(AbstractString &text, Text search, Text replacement);
  static void replace_all_into(AbstractString &output, Text input, Text search, Text replacement);

  static void pad_left(AbstractString &text, size_t total_length, Text pad = " ");
  static void pad_left_into(
      AbstractString &output, Text input, size_t total_length, Text pad = " "
  );

  static void pad_right(AbstractString &text, size_t total_length, Text pad = " ");
  static void pad_right_into(
      AbstractString &output, Text input, size_t total_length, Text pad = " "
  );

  static void remove(AbstractString &text, Text substring, int64_t count = 1);
  static void remove_into(AbstractString &output, Text input, Text substring, int64_t count = 1);

  static void remove_all(AbstractString &text, Text substring);
  static void remove_all_into(AbstractString &output, Text input, Text substring);

  static void repeat(AbstractString &text, size_t count);
  static void repeat_into(AbstractString &output, Text input, size_t count);

  static void reverse(AbstractString &text);
  static void reverse_into(AbstractString &output, Text input);

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

  static void to_string(AbstractString &output, int64_t value);
  static void to_string(AbstractString &output, uint64_t value);
  static void to_string(AbstractString &output, int32_t value);
  static void to_string(AbstractString &output, uint32_t value);
  static void to_string(AbstractString &output, int16_t value);
  static void to_string(AbstractString &output, uint16_t value);
  static void to_string(AbstractString &output, int8_t value);
  static void to_string(AbstractString &output, uint8_t value);
  static void to_string(AbstractString &output, float value);
  static void to_string(AbstractString &output, double value);
  static void to_string(AbstractString &output, bool value);

  static int64_t read_int(Text input);
  static uint64_t read_uint(Text input);
  static double read_double(Text input);

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

  static Text determine_path_separator(Text path);

private:
  static Text text_from_subslice(ConstSlice<char> slice);
};

} // namespace amelia
