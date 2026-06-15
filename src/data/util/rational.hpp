#pragma once

#include <cstdint>

#include "data/util/integer.hpp"
#include "data/util/string.hpp"

namespace amelia {

class Rational {
public:
  Rational();
  Rational(uint8_t value);
  Rational(int8_t value);
  Rational(uint16_t value);
  Rational(int16_t value);
  Rational(uint32_t value);
  Rational(int32_t value);
  Rational(uint64_t value);
  Rational(int64_t value);
  Rational(Integer value);
  Rational(Integer numerator, Integer denominator);
  explicit Rational(const String &value);
  explicit Rational(Text value);
  explicit Rational(float value);
  explicit Rational(double value);

  int8_t to_int8() const;
  uint8_t to_uint8() const;
  int16_t to_int16() const;
  uint16_t to_uint16() const;
  int32_t to_int32() const;
  uint32_t to_uint32() const;
  int64_t to_int64() const;
  uint64_t to_uint64() const;
  float to_float() const;
  double to_double() const;

  void to_fraction_string(AbstractString &output) const;
  void to_decimal_string(AbstractString &output, size_t digits_after_decimal_point = 24) const;

  void negate();
  Rational abs() const;
  Integer floor() const;
  Integer numerator() const;
  Integer denominator() const;

  Rational operator+(const Rational &other) const;
  Rational operator-(const Rational &other) const;
  Rational operator*(const Rational &other) const;
  Rational operator/(const Rational &other) const;
  Rational operator%(const Rational &other) const;

  Rational operator-() const;
  Rational operator+() const;

  Rational &operator+=(const Rational &other);
  Rational &operator-=(const Rational &other);
  Rational &operator*=(const Rational &other);
  Rational &operator/=(const Rational &other);
  Rational &operator%=(const Rational &other);

  Rational &operator++();
  Rational operator++(int);
  Rational &operator--();
  Rational operator--(int);

  bool operator==(const Rational &other) const;
  bool operator!=(const Rational &other) const;
  bool operator<(const Rational &other) const;
  bool operator<=(const Rational &other) const;
  bool operator>(const Rational &other) const;
  bool operator>=(const Rational &other) const;

private:
  void normalize();
  static void same_denom(Rational &a, Rational &b);
  Integer m_numerator;
  Integer m_denominator;
};

} // namespace amelia
