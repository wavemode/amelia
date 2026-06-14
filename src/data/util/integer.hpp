#pragma once

#include <cstdint>

#include "data/util/string.hpp"

namespace amelia {

class Integer {
public:
  Integer();
  Integer(uint8_t value);
  Integer(int8_t value);
  Integer(uint16_t value);
  Integer(int16_t value);
  Integer(uint32_t value);
  Integer(int32_t value);
  Integer(uint64_t value);
  Integer(int64_t value);
  explicit Integer(const String &value);
  explicit Integer(float value);
  explicit Integer(double value);

  Integer(const Integer &other);
  Integer(Integer &&other);
  Integer &operator=(const Integer &other);
  Integer &operator=(Integer &&other);
  ~Integer();

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

  void to_string(AbstractString &output) const;

  void negate();

  Integer abs() const;
  Integer gcd(const Integer &other) const;
  Integer lcm(const Integer &other) const;
  Integer pow(const Integer &exponent) const;

  Integer operator+(const Integer &other) const;
  Integer operator-(const Integer &other) const;
  Integer operator*(const Integer &other) const;
  Integer operator/(const Integer &other) const;
  Integer operator-() const;
  Integer operator+() const;
  Integer operator%(const Integer &other) const;
  Integer operator<<(uint32_t other) const;
  Integer operator>>(uint32_t other) const;
  Integer operator&(const Integer &other) const;
  Integer operator|(const Integer &other) const;
  Integer operator^(const Integer &other) const;
  Integer operator~() const;

  Integer &operator%=(const Integer &other);
  Integer &operator<<=(uint32_t other);
  Integer &operator>>=(uint32_t other);
  Integer &operator&=(const Integer &other);
  Integer &operator|=(const Integer &other);
  Integer &operator^=(const Integer &other);
  Integer &operator+=(const Integer &other);
  Integer &operator-=(const Integer &other);
  Integer &operator*=(const Integer &other);
  Integer &operator/=(const Integer &other);
  Integer &operator++();
  Integer operator++(int);
  Integer &operator--();
  Integer operator--(int);

  bool operator==(const Integer &other) const;
  bool operator!=(const Integer &other) const;
  bool operator<(const Integer &other) const;
  bool operator<=(const Integer &other) const;
  bool operator>(const Integer &other) const;
  bool operator>=(const Integer &other) const;

private:
  static Integer bitwise_op(
      const Integer &lhs, const Integer &rhs, uint64_t (*op)(uint64_t, uint64_t)
  );
  uint64_t extract_low_bits();
  void *m_value;
};

} // namespace amelia
