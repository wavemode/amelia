#include "NumberLiteral.h"
#include "Prelude.h"

#include <cstdint>
#include <string>

#include "data/source/NumberReadError.h"
#include "data/text/TextUtils.h"

namespace amelia {

NumberLiteral NumberLiteral::read(Text text) {
  NumberLiteral result{.has_decimal_point = false};
  int64_t base = 10;
  bool at_boundary = true;
  bool assumed_octal = false;
  bool previous_char_was_underscore = false;

  auto it = text.begin();
  if (it.peek() == '0') {
    auto base_start = it;
    it.next();
    if (it.at_end()) {
      // this zero was not the beginning of a base prefix. go back to parsing the number normally.
      it = base_start;
    } else {
      auto ch = it.peek();
      if (ch == 'x' || ch == 'X') {
        base = 16;
        it.next();
      } else if (ch == 'b' || ch == 'B') {
        base = 2;
        it.next();
      } else if (ch == 'o' || ch == 'O') {
        base = 8;
        it.next();
      } else if (TextUtils::is_digit(ch)) {
        base = 8;
      }

      if (base == 10) {
        // this zero was not the beginning of a base prefix. go back to parsing the number normally.
        it = base_start;
      } else {
        result.base_prefix = TextUtils::substr(text, base_start, it);
      }
    }
  }

  auto integer_digits_start = it;
  while (!it.at_end()) {
    auto ch = it.peek();
    signed char digit_value = -1;
    if (ch == '_') {
      if (at_boundary || previous_char_was_underscore) {
        throw NumberReadError("Underscore must separate successive digits");
      }
      previous_char_was_underscore = true;
      it.next();
    } else if (TextUtils::is_digit(ch)) {
      digit_value = ch - '0';
    } else if (ch >= 'a' && ch <= 'f') {
      digit_value = 10 + (ch - 'a');
    } else if (ch >= 'A' && ch <= 'F') {
      digit_value = 10 + (ch - 'A');
    }

    if (digit_value != -1) {
      if (digit_value >= base) {
        String err("Invalid digit '");
        err.append(ch);
        err.append("' for base ");
        TextUtils::int_to_string(err, base);
        throw NumberReadError(err);
      }
      previous_char_was_underscore = false;
      it.next();
    } else {
      break;
    }

    at_boundary = false;
  }

  if (previous_char_was_underscore) {
    throw NumberReadError("Underscore must separate successive digits");
  }
  at_boundary = true;
  previous_char_was_underscore = false;
  result.integer_digits = TextUtils::substr(text, integer_digits_start, it);

  if (!it.at_end() && it.peek() == '.') {
    it.next();
    result.has_decimal_point = true;
    if (assumed_octal) {
      // a number with a leading zero is only assumed octal if it has no decimal point or exponent
      result.base_prefix = Text();
      assumed_octal = false;
      base = 10;
    }

    if (base != 10 && base != 16) {
      throw NumberReadError("Floating point literals may only be in base 10 or 16");
    }

    auto fractional_digits_start = it;
    signed char digit_value = -1;
    while (!it.at_end()) {
      auto ch = it.peek();
      if (ch == '_') {
        if (at_boundary || previous_char_was_underscore) {
          throw NumberReadError("Underscore must separate successive digits");
        }
        previous_char_was_underscore = true;
        it.next();
      } else if (TextUtils::is_digit(ch)) {
        digit_value = ch - '0';
      } else if (ch >= 'a' && ch <= 'f') {
        digit_value = 10 + (ch - 'a');
      } else if (ch >= 'A' && ch <= 'F') {
        digit_value = 10 + (ch - 'A');
      }

      if (digit_value != -1) {
        if (digit_value >= base) {
          String err("Invalid digit '");
          err.append(ch);
          err.append("' for base ");
          TextUtils::int_to_string(err, base);
          throw NumberReadError(err);
        }
        previous_char_was_underscore = false;
        it.next();
      } else {
        break;
      }

      at_boundary = false;
    }

    result.fractional_digits = TextUtils::substr(text, fractional_digits_start, it);
  }

  if (previous_char_was_underscore) {
    throw NumberReadError("Underscore must separate successive digits");
  }
  at_boundary = true;
  previous_char_was_underscore = false;

  auto exponent_prefix_start = it;
  if (!it.at_end()) {
    auto ch = it.peek();
    if (ch == 'e' || ch == 'E') {
      if (base == 16) {
        throw NumberReadError("Hexadecimal literals must use 'p' or 'P' as the exponent prefix");
      }
      it.next();
    } else if (ch == 'p' || ch == 'P') {
      if (base != 16) {
        throw NumberReadError("Only hexadecimal literals may use 'p' or 'P' as the exponent prefix"
        );
      }
      it.next();
    }
  }
  result.exponent_prefix = TextUtils::substr(text, exponent_prefix_start, it);

  if (result.exponent_prefix.size() != 0) {
    if (assumed_octal) {
      // a number with a leading zero is only assumed octal if it has no decimal point or exponent
      result.base_prefix = Text();
      assumed_octal = false;
      base = 10;
    }

    if (base != 10 && base != 16) {
      throw NumberReadError("Only base 10 or 16 literals may have an exponent");
    }

    if (it.at_end()) {
      throw NumberReadError("Exponent has no digits");
    }

    auto exponent_sign_start = it;
    if (it.peek() == '+' || it.peek() == '-') {
      it.next();
    }
    result.exponent_sign = TextUtils::substr(text, exponent_sign_start, it);

    if (it.at_end()) {
      throw NumberReadError("Exponent has no digits");
    }

    auto exponent_digits_start = it;
    while (!it.at_end()) {
      auto ch = it.peek();
      if (ch == '_') {
        if (at_boundary || previous_char_was_underscore) {
          throw NumberReadError("Underscore must separate successive digits");
        }
        previous_char_was_underscore = true;
        it.next();
      } else if (TextUtils::is_digit(ch)) {
        previous_char_was_underscore = false;
        it.next();
      } else if (ch == '.' || TextUtils::is_alpha(ch)) {
        String err("Invalid character '");
        err.append(ch);
        err.append("' in exponent");
        throw NumberReadError(err);
      } else {
        break;
      }
      at_boundary = false;
    }

    result.exponent_digits = TextUtils::substr(text, exponent_digits_start, it);
  }

  if (previous_char_was_underscore) {
    throw NumberReadError("Underscore must separate successive digits");
  }

  return result;
}

} // namespace amelia
