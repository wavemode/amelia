#include <boost/multiprecision/cpp_int.hpp>

#include "prelude.hpp"

#include "data/util/text_utils.hpp"

#include "integer.hpp"

using boost::multiprecision::cpp_int;

namespace amelia {

Integer::Integer() : m_value(new cpp_int()) {}

Integer::Integer(uint8_t value) : m_value(new cpp_int(value)) {}

Integer::Integer(int8_t value) : m_value(new cpp_int(value)) {}

Integer::Integer(uint16_t value) : m_value(new cpp_int(value)) {}

Integer::Integer(int16_t value) : m_value(new cpp_int(value)) {}

Integer::Integer(uint32_t value) : m_value(new cpp_int(value)) {}

Integer::Integer(int32_t value) : m_value(new cpp_int(value)) {}

Integer::Integer(uint64_t value) : m_value(new cpp_int(value)) {}

Integer::Integer(int64_t value) : m_value(new cpp_int(value)) {}

Integer::Integer(const String &value) : m_value(new cpp_int()) {
  if (value.size() > 0) {
    static_cast<cpp_int *>(m_value)->operator=(cpp_int(value.c_str()));
  }
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

void Integer::to_string(AbstractString &output) const {
  output.append(Text::from(static_cast<cpp_int *>(m_value)->str().c_str()));
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

Integer Integer::pow(const Integer &exponent) const {
  Integer result(1);
  Integer exp(exponent);
  while (exp > 0) {
    result *= *this;
    exp -= 1;
  }
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
