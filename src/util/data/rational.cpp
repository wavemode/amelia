#include "rational.hpp"

#include "util/data/list.hpp"
#include "util/data/string.hpp"
#include "util/data/text_utils.hpp"

namespace amelia {

Rational::Rational() : m_numerator(0), m_denominator(1) {};

Rational::Rational(uint8_t value) : m_numerator(value), m_denominator(1) {}

Rational::Rational(int8_t value) : m_numerator(value), m_denominator(1) {}

Rational::Rational(uint16_t value) : m_numerator(value), m_denominator(1) {}

Rational::Rational(int16_t value) : m_numerator(value), m_denominator(1) {}

Rational::Rational(uint32_t value) : m_numerator(value), m_denominator(1) {}

Rational::Rational(int32_t value) : m_numerator(value), m_denominator(1) {}

Rational::Rational(uint64_t value) : m_numerator(value), m_denominator(1) {}

Rational::Rational(int64_t value) : m_numerator(value), m_denominator(1) {}

Rational::Rational(Integer value) : m_numerator(value), m_denominator(1) {}

Rational::Rational(Integer numerator, Integer denominator)
    : m_numerator(numerator), m_denominator(denominator) {
  normalize();
}

Rational::Rational(const String &value) : Rational(value.text()) {}

Rational::Rational(Text value) {
  if (TextUtils::contains(value, ".")) {
    List<Text> parts;
    TextUtils::split(parts, value, ".");

    if (parts.size() > 2) {
      String error_message = "Invalid rational: '";
      error_message.append(value);
      error_message.append("'");
      throw RuntimeError(error_message.c_str());
    }

    String digits(parts[0]);
    if (parts.size() == 2) {
      digits.append(parts[1]);
    }
    m_numerator = Integer(digits);
    if (parts.size() == 2) {
      uint8_t base = 10;
      if (TextUtils::starts_with(digits, "0b") || TextUtils::starts_with(digits, "0B")) {
        base = 2;
      } else if (TextUtils::starts_with(digits, "0x") || TextUtils::starts_with(digits, "0X")) {
        base = 16;
      } else if (TextUtils::starts_with(digits, "0o") || TextUtils::starts_with(digits, "0O") ||
                 (TextUtils::starts_with(digits, "0") && parts[0].size() > 1)) {
        base = 8;
      }
      m_denominator = Integer(base).pow(parts[1].size());
    } else {
      m_denominator = Integer(1);
    }
  } else if (TextUtils::contains(value, "/")) {
    List<Text> parts;
    TextUtils::split(parts, value, "/");

    if (parts.size() != 2) {
      String error_message = "Invalid rational: '";
      error_message.append(value);
      error_message.append("'");
      throw RuntimeError(error_message.c_str());
    }
    m_numerator = Integer(String(parts[0]));
    m_denominator = Integer(String(parts[1]));
  } else {
    m_numerator = Integer(String(value));
    m_denominator = Integer(1);
  }
  normalize();
}

Rational::Rational(float value) {
  String s;
  TextUtils::to_string(s, value);
  if (!TextUtils::contains(s.text(), ".")) {
    String error_message = "Invalid rational: '";
    error_message.append(s);
    error_message.append("'");
    throw RuntimeError(error_message.c_str());
  }
  *this = Rational(s);
}

Rational::Rational(double value) {
  String s;
  TextUtils::to_string(s, value);
  if (!TextUtils::contains(s.text(), ".")) {
    String error_message = "Invalid rational: '";
    error_message.append(s);
    error_message.append("'");
    throw RuntimeError(error_message.c_str());
  }
  *this = Rational(s);
}

int8_t Rational::to_int8() const {
  return (m_numerator / m_denominator).to_int8();
}

uint8_t Rational::to_uint8() const {
  return (m_numerator / m_denominator).to_uint8();
}

int16_t Rational::to_int16() const {
  return (m_numerator / m_denominator).to_int16();
}

uint16_t Rational::to_uint16() const {
  return (m_numerator / m_denominator).to_uint16();
}

int32_t Rational::to_int32() const {
  return (m_numerator / m_denominator).to_int32();
}

uint32_t Rational::to_uint32() const {
  return (m_numerator / m_denominator).to_uint32();
}

int64_t Rational::to_int64() const {
  return (m_numerator / m_denominator).to_int64();
}

uint64_t Rational::to_uint64() const {
  return (m_numerator / m_denominator).to_uint64();
}

float Rational::to_float() const {
  String s;
  to_decimal_string(s);
  return TextUtils::read_double(s.text());
}

double Rational::to_double() const {
  String s;
  to_decimal_string(s);
  return TextUtils::read_double(s.text());
}

// TODO: huge number abbreviation
void Rational::to_fraction_string(AbstractString &output) const {
  m_numerator.to_string(output);
  output.append("/");
  m_denominator.to_string(output);
}

void Rational::to_decimal_string(AbstractString &output, size_t max_digits_after_decimal_point)
    const {
  Rational abs_value = abs();
  Integer num = abs_value.numerator();
  Integer den = abs_value.denominator();
  Integer scaled_num = num * Integer(10).pow(max_digits_after_decimal_point);
  Integer rounded = (scaled_num * 2 + den) / (den * 2);

  if (*this < 0 && rounded != 0) {
    output.append("-");
  }

  if (max_digits_after_decimal_point == 0) {
    rounded.to_string(output);
    return;
  }

  String rounded_str;
  rounded.to_string(rounded_str);

  String padded_str;
  for (size_t i = rounded_str.size(); i < max_digits_after_decimal_point + 1; ++i) {
    padded_str.append('0');
  }
  padded_str.append(rounded_str);
  output.append(
      TextUtils::head_bytes(padded_str.text(), padded_str.size() - max_digits_after_decimal_point)
  );
  output.append(".");
  Text after_decimal = TextUtils::tail_bytes(
      padded_str.text(), padded_str.size() - max_digits_after_decimal_point
  );
  output.append(TextUtils::head_bytes(after_decimal, 1));
  output.append(TextUtils::trim_right(TextUtils::tail_bytes(after_decimal, 1), "0"));
}

void Rational::negate() {
  m_numerator.negate();
}

Rational Rational::abs() const {
  if (m_numerator < Integer(0)) {
    return Rational(-m_numerator, m_denominator);
  } else {
    return *this;
  }
}

Rational Rational::pow(uint32_t exponent) const {
  return Rational(m_numerator.pow(exponent), m_denominator.pow(exponent));
}

Integer Rational::floor() const {
  return m_numerator / m_denominator;
}

const Integer &Rational::numerator() const {
  return m_numerator;
}

const Integer &Rational::denominator() const {
  return m_denominator;
}

Rational Rational::operator+(const Rational &other) const {
  Rational lhs(*this);
  Rational rhs(other);
  same_denom(lhs, rhs);
  return Rational(lhs.m_numerator + rhs.m_numerator, lhs.m_denominator);
}

Rational Rational::operator-(const Rational &other) const {
  Rational lhs(*this);
  Rational rhs(other);
  same_denom(lhs, rhs);
  return Rational(lhs.m_numerator - rhs.m_numerator, lhs.m_denominator);
}

Rational Rational::operator*(const Rational &other) const {
  return Rational(m_numerator * other.m_numerator, m_denominator * other.m_denominator);
}

Rational Rational::operator/(const Rational &other) const {
  return Rational(m_numerator * other.m_denominator, m_denominator * other.m_numerator);
}

Rational Rational::operator%(const Rational &other) const {
  return *this - (Rational((*this / other).floor()) * other);
}

Rational Rational::operator-() const {
  return Rational(-m_numerator, m_denominator);
}

Rational Rational::operator+() const {
  return *this;
}

Rational &Rational::operator+=(const Rational &other) {
  Rational rhs(other);
  same_denom(*this, rhs);
  m_numerator += rhs.m_numerator;
  normalize();
  return *this;
}

Rational &Rational::operator-=(const Rational &other) {
  Rational rhs(other);
  same_denom(*this, rhs);
  m_numerator -= rhs.m_numerator;
  normalize();
  return *this;
}

Rational &Rational::operator*=(const Rational &other) {
  m_numerator *= other.m_numerator;
  m_denominator *= other.m_denominator;
  normalize();
  return *this;
}

Rational &Rational::operator/=(const Rational &other) {
  m_numerator *= other.m_denominator;
  m_denominator *= other.m_numerator;
  normalize();
  return *this;
}

Rational &Rational::operator++() {
  *this += Rational(1);
  return *this;
}

Rational Rational::operator++(int) {
  Rational result = *this;
  *this += Rational(1);
  return result;
}

Rational &Rational::operator--() {
  *this -= Rational(1);
  return *this;
}

Rational Rational::operator--(int) {
  Rational result = *this;
  *this -= Rational(1);
  return result;
}

bool Rational::operator==(const Rational &other) const {
  return m_numerator == other.m_numerator && m_denominator == other.m_denominator;
}

bool Rational::operator!=(const Rational &other) const {
  return !(*this == other);
}

bool Rational::operator<(const Rational &other) const {
  Rational lhs(*this);
  Rational rhs(other);
  same_denom(lhs, rhs);
  return lhs.m_numerator < rhs.m_numerator;
}

bool Rational::operator<=(const Rational &other) const {
  Rational lhs(*this);
  Rational rhs(other);
  same_denom(lhs, rhs);
  return lhs.m_numerator <= rhs.m_numerator;
}

bool Rational::operator>(const Rational &other) const {
  return !(*this <= other);
}

bool Rational::operator>=(const Rational &other) const {
  return !(*this < other);
}

void Rational::normalize() {
  if (m_denominator == 0) {
    String error_message = "Denominator cannot be zero in a Rational: ";
    String s;
    to_fraction_string(s);
    error_message.append(s.text());
    throw RuntimeError(error_message.c_str());
  }
  if (m_numerator == 0) {
    m_denominator = Integer(1);
    return;
  }
  if (m_denominator < 0) {
    m_numerator.negate();
    m_denominator.negate();
  }
  Integer gcd = m_numerator.gcd(m_denominator);
  m_numerator /= gcd;
  m_denominator /= gcd;
}

void Rational::same_denom(Rational &a, Rational &b) {
  Integer lcm = a.m_denominator.lcm(b.m_denominator);
  a.m_numerator *= lcm;
  a.m_numerator /= a.m_denominator;
  a.m_denominator = lcm;
  b.m_numerator *= lcm;
  b.m_numerator /= b.m_denominator;
  b.m_denominator = lcm;
}

} // namespace amelia
