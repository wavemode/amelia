#include "text_utils.h"

#include "prelude.h"

namespace amelia {

Text TextUtils::WHITESPACE_CHARS = " \t\n\r";

void TextUtils::split(IList<Text> &output, Text input, Text delimiter, int64_t max_splits) {
  if (delimiter.size() == 0) {
    for (CharIterator it = input.begin(); !it.at_end(); ++it) {
      output.push_back(slice(input, it, 1));
    }
    return;
  }

  auto it = input.begin();
  int64_t splits = 0;
  while (!it.at_end() && (max_splits == -1 || splits < max_splits)) {
    CharIterator found = find_after(input, delimiter, it);
    if (found.at_end()) {
      break;
    }
    output.push_back(substr(input, it, found));
    ++splits;
    it = found.plus_bytes(delimiter.size());
  }
  output.push_back(substr(input, it, input.end()));
}

void TextUtils::split(IList<String> &output, Text input, Text delimiter, int64_t max_splits) {
  List<Text> temp_output;
  split(temp_output, input, delimiter, max_splits);
  for (const Text &part : temp_output) {
    output.push_back(String(part));
  }
}

void TextUtils::join_into(IString &output, Slice<Text> parts, Text delimiter) {
  for (size_t i = 0; i < parts.size(); ++i) {
    output.append(parts[i]);
    if (i < parts.size() - 1) {
      output.append(delimiter);
    }
  }
}

Text TextUtils::trim(Text input, Text chars) {
  auto start_it = input.begin();
  while (!start_it.at_end() && !find_char(chars, start_it.peek()).at_end()) {
    ++start_it;
  }
  auto end_it = start_it;
  auto current_it = end_it;
  while (!current_it.at_end()) {
    if (find_char(chars, current_it.peek()).at_end()) {
      end_it = current_it.plus(1);
    }
    ++current_it;
  }
  return substr(input, start_it, end_it);
}
void TextUtils::trim_into(IString &output, Text input, Text chars) {
  output.append(trim(input, chars));
}

Text TextUtils::trim_left(Text input, Text chars) {
  auto start_it = input.begin();
  while (!start_it.at_end() && !find_char(chars, start_it.peek()).at_end()) {
    ++start_it;
  }
  return substr(input, start_it, input.end());
}
void TextUtils::trim_left_into(IString &output, Text input, Text chars) {
  output.append(trim_left(input, chars));
}

Text TextUtils::trim_right(Text input, Text chars) {
  auto end_it = input.begin();
  auto current_it = end_it;
  while (!current_it.at_end()) {
    if (find_char(chars, current_it.peek()).at_end()) {
      end_it = current_it.plus(1);
    }
    ++current_it;
  }
  return substr(input, input.begin(), end_it);
}
void TextUtils::trim_right_into(IString &output, Text input, Text chars) {
  output.append(trim_right(input, chars));
}

void TextUtils::to_lower(IString &text) {
  Text input = text.text();
  String output;
  for (CharIterator it = input.begin(); !it.at_end(); ++it) {
    uint32_t code_point = *it;
    if (code_point >= 'A' && code_point <= 'Z') {
      code_point += 'a' - 'A';
    }
    output.append(code_point);
  }
  text.assign(output);
}
void TextUtils::to_lower_into(IString &output, Text input) {
  for (CharIterator it = input.begin(); !it.at_end(); ++it) {
    uint32_t code_point = *it;
    if (code_point >= 'A' && code_point <= 'Z') {
      code_point += 'a' - 'A';
    }
    output.append(code_point);
  }
}

void TextUtils::to_upper(IString &text) {
  Text input = text.text();
  String output;
  for (CharIterator it = input.begin(); !it.at_end(); ++it) {
    uint32_t code_point = *it;
    if (code_point >= 'a' && code_point <= 'z') {
      code_point -= 'a' - 'A';
    }
    output.append(code_point);
  }
  text.assign(output);
}

void TextUtils::to_upper_into(IString &output, Text input) {
  for (CharIterator it = input.begin(); !it.at_end(); ++it) {
    uint32_t code_point = *it;
    if (code_point >= 'a' && code_point <= 'z') {
      code_point -= 'a' - 'A';
    }
    output.append(code_point);
  }
}

bool TextUtils::contains(Text input, Text substring) {
  if (input.size() == 0)
    return substring.size() == 0;
  return !find(input, substring).at_end();
}

bool TextUtils::starts_with(Text input, Text prefix) {
  if (input.size() == 0)
    return prefix.size() == 0;
  return find(input, prefix) == input.begin();
}

bool TextUtils::ends_with(Text input, Text suffix) {
  if (input.size() == 0)
    return suffix.size() == 0;
  size_t expected_position = input.size() - suffix.size();
  int64_t position = find_after_byte(input, suffix, expected_position);
  return position != -1 && position == expected_position;
}

CharIterator TextUtils::find(Text input, Text substring) {
  return find_after(input, substring, input.begin());
}
int64_t TextUtils::find_byte(Text input, Text substring) {
  CharIterator result = find(input, substring);
  if (substring.size() > 0 && result.at_end())
    return -1;
  return result.data().ptr() - input.data().ptr();
}

CharIterator TextUtils::find_after(Text input, Text substring, CharIterator start) {
  CharIterator start_iter = start;
  CharIterator current_iter = start_iter;
  CharIterator start_textiter = CharIterator(substring);
  CharIterator current_textiter = start_textiter;
  while (true) {
    if (current_textiter.at_end()) {
      return start_iter;
    }
    if (current_iter.at_end()) {
      return current_iter;
    }
    if (current_iter.peek() == current_textiter.peek()) {
      current_textiter.next();
      current_iter.next();
    } else {
      current_textiter = start_textiter;
      current_iter.next();
      start_iter = current_iter;
    }
  }
}

int64_t TextUtils::find_after_byte(Text input, Text substring, size_t index_start) {
  CharIterator result = find_after(input, substring, input.begin().plus_bytes(index_start));
  if (substring.size() > 0 && result.at_end()) {
    return -1;
  }
  return result.data().ptr() - input.data().ptr();
}

CharIterator TextUtils::find_before(Text input, Text substring, CharIterator end) {
  if (substring.size() == 0)
    return end;
  CharIterator start_iter = input.begin();
  CharIterator current_iter = start_iter;
  CharIterator start_textiter = CharIterator(substring);
  CharIterator current_textiter = start_textiter;
  CharIterator last_found = input.end();
  while (true) {
    if (current_textiter.at_end()) {
      last_found = start_iter;
      current_textiter = start_textiter;
      continue;
    }
    if (current_iter.at_end() || current_iter == end) {
      return last_found;
    }
    if (current_iter.peek() == current_textiter.peek()) {
      current_textiter.next();
      current_iter.next();
    } else {
      current_textiter = start_textiter;
      current_iter.next();
      start_iter = current_iter;
    }
  }
}

int64_t TextUtils::find_before_byte(Text input, Text substring, size_t index_end) {
  CharIterator result = find_before(input, substring, input.begin().plus_bytes(index_end));
  if (substring.size() > 0 && result.at_end()) {
    return -1;
  }
  return result.data().ptr() - input.data().ptr();
}

CharIterator TextUtils::find_char(Text input, uint32_t code_point) {
  CharIterator it = input.begin();
  while (!it.at_end()) {
    if (*it == code_point) {
      break;
    }
    ++it;
  }
  return it;
}

int64_t TextUtils::find_char_byte(Text input, uint32_t code_point) {
  CharIterator it = find_char(input, code_point);
  if (it.at_end()) {
    return -1;
  }
  return it.data().ptr() - input.data().ptr();
}

CharIterator TextUtils::find_char_after(Text input, uint32_t code_point, CharIterator start) {
  CharIterator it = start;
  while (!it.at_end()) {
    if (*it == code_point) {
      break;
    }
    ++it;
  }
  return it;
}
int64_t TextUtils::find_char_after_byte(Text input, uint32_t code_point, size_t index_start) {
  CharIterator it = find_char_after(input, code_point, input.begin().plus_bytes(index_start));
  if (it.at_end()) {
    return -1;
  }
  return it.data().ptr() - input.data().ptr();
}

CharIterator TextUtils::find_char_before(Text input, uint32_t code_point, CharIterator end) {
  CharIterator it = input.begin();
  CharIterator last_found = input.end();
  while (!it.at_end() && it != end) {
    if (*it == code_point) {
      last_found = it;
    }
    ++it;
  }
  return last_found;
}
int64_t TextUtils::find_char_before_byte(Text input, uint32_t code_point, size_t index_end) {
  CharIterator it = find_char_before(input, code_point, input.begin().plus_bytes(index_end));
  if (it.at_end()) {
    return -1;
  }
  return it.data().ptr() - input.data().ptr();
}

void TextUtils::find_all(IList<CharIterator> &output, Text input, Text substring) {
  CharIterator it = input.begin();
  while (!it.at_end()) {
    CharIterator found = find_after(input, substring, it);
    if (found.at_end()) {
      break;
    }
    output.push_back(found);
    it = substring.size() == 0 ? found.plus(1) : found.plus_bytes(substring.size());
  }
}
void TextUtils::find_all_bytes(IList<int64_t> &output, Text input, Text substring) {
  CharIterator it = input.begin();
  while (!it.at_end()) {
    CharIterator found = find_after(input, substring, it);
    if (found.at_end()) {
      break;
    }
    output.push_back(found.data().ptr() - input.data().ptr());
    it = substring.size() == 0 ? found.plus(1) : found.plus_bytes(substring.size());
  }
}

void TextUtils::find_all_chars(IList<CharIterator> &output, Text input, uint32_t code_point) {
  CharIterator it = input.begin();
  while (!it.at_end()) {
    CharIterator found = find_char_after(input, code_point, it);
    if (found.at_end()) {
      break;
    }
    output.push_back(found);
    it = found.plus(1);
  }
}

void TextUtils::find_all_char_bytes(IList<int64_t> &output, Text input, uint32_t code_point) {
  CharIterator it = input.begin();
  while (!it.at_end()) {
    CharIterator found = find_char_after(input, code_point, it);
    if (found.at_end()) {
      break;
    }
    output.push_back(found.data().ptr() - input.data().ptr());
    it = found.plus(1);
  }
}

void TextUtils::replace(IString &text, Text search, Text replacement, int64_t count) {
  String output;
  replace_into(output, text.text(), search, replacement, count);
  text.assign(output);
}
void TextUtils::replace_into(
    IString &output, Text input, Text search, Text replacement, int64_t count
) {
  CharIterator it = input.begin();
  int64_t replacements = 0;
  while (count == -1 || replacements < count) {
    if (search.size() == 0) {
      output.append(replacement);
      if (it.at_end()) {
        break;
      }
      output.append(*it);
      ++it;
      ++replacements;
      continue;
    }
    if (it.at_end()) {
      break;
    }
    CharIterator found = find_after(input, search, it);
    if (found.at_end()) {
      break;
    }
    output.append(substr(input, it, found));
    output.append(replacement);
    ++replacements;
    it = found.plus_bytes(search.size());
  }
  output.append(substr(input, it, input.end()));
}

void TextUtils::replace_all(IString &text, Text search, Text replacement) {
  replace(text, search, replacement, -1);
}
void TextUtils::replace_all_into(IString &output, Text input, Text search, Text replacement) {
  replace_into(output, input, search, replacement, -1);
}

void TextUtils::pad_left(IString &text, size_t total_length, Text pad) {
  Text input = text.text();
  String output;
  pad_left_into(output, input, total_length, pad);
  text.assign(output);
}

void TextUtils::pad_left_into(IString &output, Text input, size_t total_length, Text pad) {
  size_t input_length = count_chars(input);
  size_t pad_length = count_chars(pad);
  if (input_length < total_length && pad_length > 0) {
    size_t total_pad_needed = total_length - input_length;
    size_t full_repeats = total_pad_needed / pad_length;
    size_t remainder = total_pad_needed % pad_length;
    for (size_t i = 0; i < full_repeats; ++i) {
      output.append(pad);
    }
    if (remainder) {
      output.append(pad);
    }
  }
  output.append(input);
}

void TextUtils::pad_right(IString &text, size_t total_length, Text pad) {
  String output;
  pad_right_into(output, text.text(), total_length, pad);
  text.assign(output);
}
void TextUtils::pad_right_into(IString &output, Text input, size_t total_length, Text pad) {
  size_t input_length = count_chars(input);
  size_t pad_length = count_chars(pad);
  output.append(input);
  if (input_length < total_length && pad_length > 0) {
    size_t total_pad_needed = total_length - input_length;
    size_t full_repeats = total_pad_needed / pad_length;
    size_t remainder = total_pad_needed % pad_length;
    for (size_t i = 0; i < full_repeats; ++i) {
      output.append(pad);
    }
    if (remainder) {
      output.append(pad);
    }
  }
}

void TextUtils::remove(IString &text, Text substring, int64_t count) {
  String output;
  remove_into(output, text.text(), substring, count);
  text.assign(output);
}
void TextUtils::remove_into(IString &output, Text input, Text substring, int64_t count) {
  replace_into(output, input, substring, "", count);
}

void TextUtils::remove_all(IString &text, Text substring) {
  String output;
  remove_all_into(output, text.text(), substring);
  text.assign(output);
}
void TextUtils::remove_all_into(IString &output, Text input, Text substring) {
  replace_into(output, input, substring, "", -1);
}

void TextUtils::repeat(IString &text, size_t count) {
  if (count == 0) {
    text.assign("");
    return;
  }
  String output;
  repeat_into(output, text.text(), count);
  text.assign(output);
}
void TextUtils::repeat_into(IString &output, Text input, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    output.append(input);
  }
}

void TextUtils::reverse(IString &text) {
  Text input = text.text();
  String output;
  reverse_into(output, input);
  text.assign(output);
}
void TextUtils::reverse_into(IString &output, Text input) {
  List<uint32_t> code_points;
  for (uint32_t ch : input) {
    code_points.push_back(ch);
  }
  for (size_t i = code_points.size(); i > 0; --i) {
    output.append(code_points[i - 1]);
  }
}

Text TextUtils::strip_prefix(Text input, Text prefix) {
  if (starts_with(input, prefix)) {
    return tail_bytes(input, prefix.size());
  }
  return input;
}

Text TextUtils::strip_suffix(Text input, Text suffix) {
  if (ends_with(input, suffix)) {
    return head_bytes(input, input.size() - suffix.size());
  }
  return input;
}

Text TextUtils::slice(Text input, CharIterator char_start, size_t char_length) {
  auto char_end = char_start;
  while (!char_end.at_end() && char_length > 0) {
    ++char_end;
    --char_length;
  }
  return substr(input, char_start, char_end);
}

Text TextUtils::slice_bytes(Text input, size_t index_start, size_t byte_length) {
  if (index_start >= input.size()) {
    index_start = input.size() - 1;
  }
  if (index_start + byte_length > input.size()) {
    byte_length = input.size() - index_start;
  }
  return text_from_subslice(Slice(input.data().ptr() + index_start, byte_length));
}

Text TextUtils::substr(Text input, CharIterator char_start, CharIterator char_end) {
  auto input_ptr = input.data().ptr();
  auto start_ptr = char_start.data().ptr();
  size_t start_offset = start_ptr - input_ptr;
  auto end_ptr = char_end.data().ptr();
  size_t end_offset = end_ptr - input_ptr;
  if (end_offset > input.size()) {
    end_offset = input.size();
  }
  if (start_offset > end_offset) {
    start_offset = end_offset;
  }
  return text_from_subslice(Slice(start_ptr, end_offset - start_offset));
}

Text TextUtils::substr_bytes(Text input, size_t index_start, size_t index_end) {
  if (index_end > input.size()) {
    index_end = input.size();
  }
  if (index_start > index_end) {
    index_start = index_end;
  }
  return text_from_subslice(Slice(input.data().ptr() + index_start, index_end - index_start));
}

Text TextUtils::head(Text input, CharIterator char_end) {
  return substr(input, input.begin(), char_end);
}

Text TextUtils::head_bytes(Text input, size_t index_end) {
  return substr_bytes(input, 0, index_end);
}

Text TextUtils::tail(Text input, CharIterator char_start) {
  return substr(input, char_start, input.end());
}

Text TextUtils::tail_bytes(Text input, size_t index_start) {
  return substr_bytes(input, index_start, input.size());
}

void TextUtils::to_string(IString &output, int64_t value) {
  std::string s1 = std::to_string(value);
  output.append(Text::from(s1));
}

void TextUtils::to_string(IString &output, size_t value) {
  std::string s1 = std::to_string(value);
  output.append(Text::from(s1));
}

void TextUtils::to_string(IString &output, double value) {
  std::string s1 = std::to_string(value);
  output.append(Text::from(s1));
}

bool TextUtils::is_digit(uint32_t ch) { return ch >= '0' && ch <= '9'; }
bool TextUtils::is_digit(Text input) {
  if (input.size() == 0) {
    return false;
  }
  for (auto ch : input) {
    if (!is_digit(ch)) {
      return false;
    }
  }
  return true;
}

bool TextUtils::is_alpha(uint32_t ch) {
  return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}
bool TextUtils::is_alpha(Text input) {
  if (input.size() == 0) {
    return false;
  }
  for (auto ch : input) {
    if (!is_alpha(ch)) {
      return false;
    }
  }
  return true;
}

bool TextUtils::is_alnum(uint32_t ch) { return is_alpha(ch) || is_digit(ch); }
bool TextUtils::is_alnum(Text input) {
  if (input.size() == 0) {
    return false;
  }
  for (auto ch : input) {
    if (!is_alnum(ch)) {
      return false;
    }
  }
  return true;
}

bool TextUtils::is_ascii(uint32_t ch) { return ch <= 127; }
bool TextUtils::is_ascii(Text input) {
  for (auto ch : input) {
    if (!is_ascii(ch)) {
      return false;
    }
  }
  return true;
}

Text TextUtils::text_from_subslice(Slice<const char> slice) {
  if (slice.size() > 0) {
    // raise an exception if this slice is not at a valid code point boundary
    CharIterator(slice).peek();
  }
  Text result;
  result.m_slice = slice;
  return result;
}

size_t TextUtils::count_chars(Text input) {
  size_t char_count = 0;
  for (CharIterator it = input.begin(); !it.at_end(); ++it) {
    ++char_count;
  }
  return char_count;
}
size_t TextUtils::count(Text input, Text substring) {
  if (substring.size() == 0) {
    return count_chars(input) + 1;
  }
  size_t count = 0;
  CharIterator it = input.begin();
  while (!it.at_end()) {
    CharIterator found = find_after(input, substring, it);
    if (found.at_end()) {
      break;
    }
    ++count;
    it = found.plus_bytes(substring.size());
  }
  return count;
}

} // namespace amelia
