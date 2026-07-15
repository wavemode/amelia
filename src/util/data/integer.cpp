#include <boost/multiprecision/cpp_int.hpp>

#include "integer.hpp"

#include "util/data/string.hpp"
#include "util/data/char_iterator.hpp"
#include "util/data/text_utils.hpp"

using boost::multiprecision::cpp_int;

namespace amelia {

Integer binary_string_to_integer(Text binary_string) {
  Integer result;
  for (char c : binary_string) {
    if (c == '0') {
      result <<= 1;
    } else if (c == '1') {
      result <<= 1;
      result += 1;
    } else {
      String error_message = "Invalid binary literal: '";
      error_message.append(binary_string);
      error_message.append("'");
      throw RuntimeError(error_message.c_str());
    }
  }
  return result;
}

Integer octal_string_to_integer(Text octal_string) {
  Integer result;
  for (char c : octal_string) {
    if (c >= '0' && c <= '7') {
      result <<= 3;
      result += (c - '0');
    } else {
      String error_message = "Invalid octal literal: '";
      error_message.append(octal_string);
      error_message.append("'");
      throw RuntimeError(error_message.c_str());
    }
  }
  return result;
}

Integer hexadecimal_string_to_integer(Text hexadecimal_string) {
  Integer result;
  for (char c : hexadecimal_string) {
    if (c >= '0' && c <= '9') {
      result <<= 4;
      result += (c - '0');
    } else if (c >= 'a' && c <= 'f') {
      result <<= 4;
      result += (c - 'a' + 10);
    } else if (c >= 'A' && c <= 'F') {
      result <<= 4;
      result += (c - 'A' + 10);
    } else {
      String error_message = "Invalid hexadecimal literal: '";
      error_message.append(hexadecimal_string);
      error_message.append("'");
      throw RuntimeError(error_message.c_str());
    }
  }
  return result;
}

Integer::Integer() : m_value(new cpp_int()) {}

Integer::Integer(uint8_t value) : m_value(new cpp_int(value)) {}

Integer::Integer(int8_t value) : m_value(new cpp_int(value)) {}

Integer::Integer(uint16_t value) : m_value(new cpp_int(value)) {}

Integer::Integer(int16_t value) : m_value(new cpp_int(value)) {}

Integer::Integer(uint32_t value) : m_value(new cpp_int(value)) {}

Integer::Integer(int32_t value) : m_value(new cpp_int(value)) {}

Integer::Integer(uint64_t value) : m_value(new cpp_int(value)) {}

Integer::Integer(int64_t value) : m_value(new cpp_int(value)) {}

Integer::Integer(const String &value) : Integer(value.text()) {}

Integer::Integer(Text text_value) : m_value(nullptr) {
  Integer result;
  if (text_value.size() > 0) {
    if (TextUtils::starts_with(text_value, "0b") || TextUtils::starts_with(text_value, "0B")) {
      result = binary_string_to_integer(TextUtils::tail_bytes(text_value, 2));
    } else if (TextUtils::starts_with(text_value, "0o") ||
               TextUtils::starts_with(text_value, "0O")) {
      result = octal_string_to_integer(TextUtils::tail_bytes(text_value, 2));
    } else if (TextUtils::starts_with(text_value, "0x") ||
               TextUtils::starts_with(text_value, "0X")) {
      result = hexadecimal_string_to_integer(TextUtils::tail_bytes(text_value, 2));
    } else if (TextUtils::starts_with(text_value, "0") && text_value.size() > 1) {
      result = octal_string_to_integer(TextUtils::tail_bytes(text_value, 1));
    } else {
      String str_value(text_value);
      try {
        static_cast<cpp_int *>(result.m_value)->operator=(cpp_int(str_value.c_str()));
      } catch (...) {
        String error_message = "Invalid integer literal: '";
        error_message.append(str_value);
        error_message.append("'");
        throw RuntimeError(error_message.c_str());
      }
    }
  }
  m_value = result.m_value;
  result.m_value = nullptr;
}

Integer::Integer(float value) : m_value(new cpp_int(value)) {}

Integer::Integer(double value) : m_value(new cpp_int(value)) {}

Integer::Integer(const Integer &other)
    : m_value(new cpp_int(*static_cast<cpp_int *>(other.m_value))) {}

Integer::Integer(Integer &&other) : m_value(other.m_value) {
  other.m_value = new cpp_int();
}

Integer &Integer::operator=(const Integer &other) {
  if (this != &other) {
    static_cast<cpp_int *>(m_value)->operator=(*static_cast<cpp_int *>(other.m_value));
  }
  return *this;
}

Integer &Integer::operator=(Integer &&other) {
  if (this != &other) {
    static_cast<cpp_int *>(m_value)->operator=(move(*static_cast<cpp_int *>(other.m_value)));
  }
  return *this;
}

Integer::~Integer() {
  delete static_cast<cpp_int *>(m_value);
}

int8_t Integer::to_int8() const {
  if (*this < INT8_MIN || *this > INT8_MAX) {
    String error_message = "Integer value out of range for int8: ";
    String s;
    to_string(s);
    error_message.append(s.text());
    throw RuntimeError(error_message.c_str());
  }
  return static_cast<int8_t>(*static_cast<cpp_int *>(m_value));
}

uint8_t Integer::to_uint8() const {
  if (*this < 0 || *this > UINT8_MAX) {
    String error_message = "Integer value out of range for uint8: ";
    String s;
    to_string(s);
    error_message.append(s.text());
    throw RuntimeError(error_message.c_str());
  }
  return static_cast<uint8_t>(*static_cast<cpp_int *>(m_value));
}

int16_t Integer::to_int16() const {
  if (*this < INT16_MIN || *this > INT16_MAX) {
    String error_message = "Integer value out of range for int16: ";
    String s;
    to_string(s);
    error_message.append(s.text());
    throw RuntimeError(error_message.c_str());
  }
  return static_cast<int16_t>(*static_cast<cpp_int *>(m_value));
}

uint16_t Integer::to_uint16() const {
  if (*this < 0 || *this > UINT16_MAX) {
    String error_message = "Integer value out of range for uint16: ";
    String s;
    to_string(s);
    error_message.append(s.text());
    throw RuntimeError(error_message.c_str());
  }
  return static_cast<uint16_t>(*static_cast<cpp_int *>(m_value));
}

int32_t Integer::to_int32() const {
  if (*this < INT32_MIN || *this > INT32_MAX) {
    String error_message = "Integer value out of range for int32: ";
    String s;
    to_string(s);
    error_message.append(s.text());
    throw RuntimeError(error_message.c_str());
  }
  return static_cast<int32_t>(*static_cast<cpp_int *>(m_value));
}

uint32_t Integer::to_uint32() const {
  if (*this < 0 || *this > UINT32_MAX) {
    String error_message = "Integer value out of range for uint32: ";
    String s;
    to_string(s);
    error_message.append(s.text());
    throw RuntimeError(error_message.c_str());
  }
  return static_cast<uint32_t>(*static_cast<cpp_int *>(m_value));
}

int64_t Integer::to_int64() const {
  if (*this < INT64_MIN || *this > INT64_MAX) {
    String error_message = "Integer value out of range for int64: ";
    String s;
    to_string(s);
    error_message.append(s.text());
    throw RuntimeError(error_message.c_str());
  }
  return static_cast<int64_t>(*static_cast<cpp_int *>(m_value));
}

uint64_t Integer::to_uint64() const {
  if (*this < 0 || *this > UINT64_MAX) {
    String error_message = "Integer value out of range for uint64: ";
    String s;
    to_string(s);
    error_message.append(s.text());
    throw RuntimeError(error_message.c_str());
  }
  return static_cast<uint64_t>(*static_cast<cpp_int *>(m_value));
}

float Integer::to_float() const {
  return static_cast<float>(*static_cast<cpp_int *>(m_value));
}

double Integer::to_double() const {
  return static_cast<double>(*static_cast<cpp_int *>(m_value));
}

// TODO: huge number abbreviation
void Integer::to_string(AbstractString &output) const {
  output.append(Text::from(static_cast<cpp_int *>(m_value)->str().c_str()));
}

void Integer::to_binary_string(AbstractString &output) const {
  Integer value = *this;
  if (value < 0) {
    value.negate();
    output.append('-');
  }
  List<char> digits;
  do {
    digits.push_back((value % 2).to_int8() + '0');
    value /= 2;
  } while (value > 0);
  for (size_t i = digits.size(); i > 0; --i) {
    output.append(digits[i - 1]);
  }
}

void Integer::to_octal_string(AbstractString &output) const {
  Integer value = *this;
  if (value < 0) {
    value.negate();
    output.append('-');
  }
  List<char> digits;
  do {
    digits.push_back((value % 8).to_int8() + '0');
    value /= 8;
  } while (value > 0);
  for (size_t i = digits.size(); i > 0; --i) {
    output.append(digits[i - 1]);
  }
}

void Integer::to_hex_string(AbstractString &output) const {
  Integer value = *this;
  if (value < 0) {
    value.negate();
    output.append('-');
  }
  List<char> digits;
  do {
    int8_t digit = (value % 16).to_int8();
    if (digit < 10) {
      digits.push_back(digit + '0');
    } else {
      digits.push_back(digit - 10 + 'A');
    }
    value /= 16;
  } while (value > 0);
  for (size_t i = digits.size(); i > 0; --i) {
    output.append(digits[i - 1]);
  }
}

void Integer::negate() {
  *this = -*this;
}

Integer Integer::abs() const {
  if (*this < 0) {
    return -*this;
  } else {
    return *this;
  }
}

Integer Integer::gcd(const Integer &other) const {
  if (*this == 0) {
    return other.abs();
  }

  Integer a(abs());
  Integer b(other.abs());

  while (b != 0) {
    Integer temp = b;
    b = a % b;
    a = move(temp);
  }

  return a;
}

Integer Integer::lcm(const Integer &other) const {
  return (abs() * other.abs()) / gcd(other);
}

Integer Integer::pow(uint32_t exponent) const {
  Integer result;
  static_cast<cpp_int *>(result.m_value)
      ->operator=(boost::multiprecision::pow(*static_cast<cpp_int *>(m_value), exponent));
  return result;
}

Integer Integer::operator+(const Integer &other) const {
  Integer result(*this);
  static_cast<cpp_int *>(result.m_value)->operator+=(*static_cast<cpp_int *>(other.m_value));
  return result;
}

Integer Integer::operator-(const Integer &other) const {
  Integer result(*this);
  static_cast<cpp_int *>(result.m_value)->operator-=(*static_cast<cpp_int *>(other.m_value));
  return result;
}

Integer Integer::operator*(const Integer &other) const {
  Integer result(*this);
  static_cast<cpp_int *>(result.m_value)->operator*=(*static_cast<cpp_int *>(other.m_value));
  return result;
}

Integer Integer::operator/(const Integer &other) const {
  Integer result(*this);
  static_cast<cpp_int *>(result.m_value)->operator/=(*static_cast<cpp_int *>(other.m_value));
  return result;
}

Integer Integer::operator-() const {
  Integer result(*this);
  static_cast<cpp_int *>(result.m_value)->operator*=(-1);
  return result;
}

Integer Integer::operator+() const {
  return *this;
}

Integer Integer::operator%(const Integer &other) const {
  Integer result(*this);
  static_cast<cpp_int *>(result.m_value)->operator%=(*static_cast<cpp_int *>(other.m_value));
  return result;
}

Integer Integer::operator<<(unsigned other) const {
  Integer result(*this);
  static_cast<cpp_int *>(result.m_value)->operator<<=(other);
  return result;
}

Integer Integer::operator>>(uint32_t other) const {
  Integer result(*this);
  static_cast<cpp_int *>(result.m_value)->operator>>=(other);
  return result;
}

Integer Integer::operator&(const Integer &other) const {
  Integer result(*this);
  static_cast<cpp_int *>(result.m_value)->operator&=(*static_cast<cpp_int *>(other.m_value));
  return result;
}

Integer Integer::operator|(const Integer &other) const {
  Integer result(*this);
  static_cast<cpp_int *>(result.m_value)->operator|=(*static_cast<cpp_int *>(other.m_value));
  return result;
}

Integer Integer::operator^(const Integer &other) const {
  Integer result(*this);
  static_cast<cpp_int *>(result.m_value)->operator^=(*static_cast<cpp_int *>(other.m_value));
  return result;
}

Integer Integer::operator~() const {
  return -*this - 1;
}

Integer &Integer::operator%=(const Integer &other) {
  static_cast<cpp_int *>(m_value)->operator%=(*static_cast<cpp_int *>(other.m_value));
  return *this;
}

Integer &Integer::operator<<=(uint32_t other) {
  static_cast<cpp_int *>(m_value)->operator<<=(other);
  return *this;
}

Integer &Integer::operator>>=(uint32_t other) {
  static_cast<cpp_int *>(m_value)->operator>>=(other);
  return *this;
}

Integer &Integer::operator&=(const Integer &other) {
  static_cast<cpp_int *>(m_value)->operator&=(*static_cast<cpp_int *>(other.m_value));
  return *this;
}

Integer &Integer::operator|=(const Integer &other) {
  static_cast<cpp_int *>(m_value)->operator|=(*static_cast<cpp_int *>(other.m_value));
  return *this;
}

Integer &Integer::operator^=(const Integer &other) {
  static_cast<cpp_int *>(m_value)->operator^=(*static_cast<cpp_int *>(other.m_value));
  return *this;
}

Integer &Integer::operator+=(const Integer &other) {
  static_cast<cpp_int *>(m_value)->operator+=(*static_cast<cpp_int *>(other.m_value));
  return *this;
}

Integer &Integer::operator-=(const Integer &other) {
  static_cast<cpp_int *>(m_value)->operator-=(*static_cast<cpp_int *>(other.m_value));
  return *this;
}

Integer &Integer::operator*=(const Integer &other) {
  static_cast<cpp_int *>(m_value)->operator*=(*static_cast<cpp_int *>(other.m_value));
  return *this;
}

Integer &Integer::operator/=(const Integer &other) {
  static_cast<cpp_int *>(m_value)->operator/=(*static_cast<cpp_int *>(other.m_value));
  return *this;
}

Integer &Integer::operator++() {
  return *this += 1;
}

Integer Integer::operator++(int) {
  Integer result = *this;
  *this += 1;
  return result;
}

Integer &Integer::operator--() {
  return *this -= 1;
}

Integer Integer::operator--(int) {
  Integer result = *this;
  *this -= 1;
  return result;
}

bool Integer::operator==(const Integer &other) const {
  return (*static_cast<const cpp_int *>(m_value)) == (*static_cast<const cpp_int *>(other.m_value));
}
bool Integer::operator!=(const Integer &other) const {
  return !(*this == other);
}
bool Integer::operator<(const Integer &other) const {
  return (*static_cast<const cpp_int *>(m_value)) < (*static_cast<const cpp_int *>(other.m_value));
}
bool Integer::operator<=(const Integer &other) const {
  return (*static_cast<const cpp_int *>(m_value)) <= (*static_cast<const cpp_int *>(other.m_value));
}
bool Integer::operator>(const Integer &other) const {
  return !(*this <= other);
}
bool Integer::operator>=(const Integer &other) const {
  return !(*this < other);
}

} // namespace amelia
