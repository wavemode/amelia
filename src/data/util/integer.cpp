#include <BigInt.hpp>

#include "prelude.hpp"

#include "data/util/text_utils.hpp"

#include "integer.hpp"

namespace amelia {

namespace {

const Integer TWO_64 = Integer(1) << 64;

uint64_t bitwise_and_words(uint64_t a, uint64_t b) {
  return a & b;
}

uint64_t bitwise_or_words(uint64_t a, uint64_t b) {
  return a | b;
}

uint64_t bitwise_xor_words(uint64_t a, uint64_t b) {
  return a ^ b;
}

bool highest_bit_is_set(uint64_t word) {
  return word & (1ULL << 63);
}

} // namespace

Integer::Integer() : m_value(new BigInt()) {}

Integer::Integer(uint8_t value) : m_value(new BigInt(value)) {}

Integer::Integer(int8_t value) : m_value(new BigInt(value)) {}

Integer::Integer(uint16_t value) : m_value(new BigInt(value)) {}

Integer::Integer(int16_t value) : m_value(new BigInt(value)) {}

Integer::Integer(uint32_t value) : m_value(new BigInt(value)) {}

Integer::Integer(int32_t value) : m_value(new BigInt(value)) {}

Integer::Integer(uint64_t value) {
  String s;
  TextUtils::to_string(s, value);
  m_value = new BigInt(s.c_str());
}

Integer::Integer(int64_t value) : m_value(new BigInt(value)) {}

Integer::Integer(const String &value) : m_value(new BigInt()) {
  if (value.size() > 0) {
    static_cast<BigInt *>(m_value)->operator=(BigInt(value.c_str()));
  }
}

Integer::Integer(float value) {
  String s;
  TextUtils::to_string(s, value);
  List<String> parts;
  TextUtils::split(parts, s, ".");
  m_value = new BigInt(parts[0].c_str());
}

Integer::Integer(double value) {
  String s;
  TextUtils::to_string(s, value);
  List<String> parts;
  TextUtils::split(parts, s, ".");
  m_value = new BigInt(parts[0].c_str());
}

Integer::Integer(const Integer &other)
    : m_value(new BigInt(*static_cast<BigInt *>(other.m_value))) {}

Integer::Integer(Integer &&other) : m_value(other.m_value) {
  other.m_value = nullptr;
}

Integer &Integer::operator=(const Integer &other) {
  if (this != &other) {
    static_cast<BigInt *>(m_value)->operator=(*static_cast<BigInt *>(other.m_value));
  }
  return *this;
}

Integer &Integer::operator=(Integer &&other) {
  if (this != &other) {
    static_cast<BigInt *>(m_value)->operator=(*static_cast<BigInt *>(other.m_value));
  }
  return *this;
}

Integer::~Integer() {
  delete static_cast<BigInt *>(m_value);
}

int8_t Integer::to_int8() const {
  auto result = static_cast<BigInt *>(m_value)->to_long_long();
  if (result < INT8_MIN || result > INT8_MAX) {
    String error_message = "Integer value out of range for int8: ";
    String s;
    to_string(s);
    error_message.append(s.text());
    throw RuntimeError(error_message.c_str());
  }
  return static_cast<int8_t>(result);
}

uint8_t Integer::to_uint8() const {
  auto result = static_cast<BigInt *>(m_value)->to_long_long();
  if (result < 0 || result > UINT8_MAX) {
    String error_message = "Integer value out of range for uint8: ";
    String s;
    to_string(s);
    error_message.append(s.text());
    throw RuntimeError(error_message.c_str());
  }
  return static_cast<uint8_t>(result);
}

int16_t Integer::to_int16() const {
  auto result = static_cast<BigInt *>(m_value)->to_long_long();
  if (result < INT16_MIN || result > INT16_MAX) {
    String error_message = "Integer value out of range for int16: ";
    String s;
    to_string(s);
    error_message.append(s.text());
    throw RuntimeError(error_message.c_str());
  }
  return static_cast<int16_t>(result);
}

uint16_t Integer::to_uint16() const {
  auto result = static_cast<BigInt *>(m_value)->to_long_long();
  if (result < 0 || result > UINT16_MAX) {
    String error_message = "Integer value out of range for uint16: ";
    String s;
    to_string(s);
    error_message.append(s.text());
    throw RuntimeError(error_message.c_str());
  }
  return static_cast<uint16_t>(result);
}

int32_t Integer::to_int32() const {
  auto result = static_cast<BigInt *>(m_value)->to_long_long();
  if (result < INT32_MIN || result > INT32_MAX) {
    String error_message = "Integer value out of range for int32: ";
    String s;
    to_string(s);
    error_message.append(s.text());
    throw RuntimeError(error_message.c_str());
  }
  return static_cast<int32_t>(result);
}

uint32_t Integer::to_uint32() const {
  auto result = static_cast<BigInt *>(m_value)->to_long_long();
  if (result < 0 || result > UINT32_MAX) {
    String error_message = "Integer value out of range for uint32: ";
    String s;
    to_string(s);
    error_message.append(s.text());
    throw RuntimeError(error_message.c_str());
  }
  return static_cast<uint32_t>(result);
}

int64_t Integer::to_int64() const {
  return static_cast<BigInt *>(m_value)->to_long_long();
}

uint64_t Integer::to_uint64() const {
  auto s = static_cast<BigInt *>(m_value)->to_string();
  return TextUtils::read_uint(Text::from(s.c_str()));
}

float Integer::to_float() const {
  return static_cast<float>(to_double());
}

double Integer::to_double() const {
  return static_cast<double>(static_cast<BigInt *>(m_value)->to_long_long());
}

void Integer::to_string(AbstractString &output) const {
  auto s = static_cast<BigInt *>(m_value)->to_string();
  output.append(Text::from(s.c_str()));
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
  Integer result;
  static_cast<BigInt *>(result.m_value)
      ->operator=(::gcd(*static_cast<BigInt *>(m_value), *static_cast<BigInt *>(other.m_value)));
  return result;
}

Integer Integer::lcm(const Integer &other) const {
  Integer result;
  static_cast<BigInt *>(result.m_value)
      ->operator=(::lcm(*static_cast<BigInt *>(m_value), *static_cast<BigInt *>(other.m_value)));
  return result;
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
  static_cast<BigInt *>(result.m_value)->operator+=(*static_cast<BigInt *>(other.m_value));
  return result;
}

Integer Integer::operator-(const Integer &other) const {
  Integer result(*this);
  static_cast<BigInt *>(result.m_value)->operator-=(*static_cast<BigInt *>(other.m_value));
  return result;
}

Integer Integer::operator*(const Integer &other) const {
  Integer result(*this);
  static_cast<BigInt *>(result.m_value)->operator*=(*static_cast<BigInt *>(other.m_value));
  return result;
}

Integer Integer::operator/(const Integer &other) const {
  Integer result(*this);
  static_cast<BigInt *>(result.m_value)->operator/=(*static_cast<BigInt *>(other.m_value));
  return result;
}

Integer Integer::operator-() const {
  Integer result(*this);
  static_cast<BigInt *>(result.m_value)->operator*=(-1);
  return result;
}

Integer Integer::operator+() const {
  return *this;
}

Integer Integer::operator%(const Integer &other) const {
  Integer result(*this);
  static_cast<BigInt *>(result.m_value)->operator%=(*static_cast<BigInt *>(other.m_value));
  return result;
}

Integer Integer::operator<<(unsigned other) const {
  Integer result(*this);
  while (other > 0) {
    result *= 2;
    --other;
  }
  return result;
}

Integer Integer::operator>>(uint32_t other) const {
  Integer result(*this);
  if (*this >= 0) {
    while (other > 0) {
      result /= 2;
      --other;
    }
  } else {
    Integer one(-1);
    while (other > 0) {
      result = (result - one) / 2 + one;
      --other;
    }
  }
  return result;
}

Integer Integer::operator&(const Integer &other) const {
  return bitwise_op(*this, other, bitwise_and_words);
}

Integer Integer::operator|(const Integer &other) const {
  return bitwise_op(*this, other, bitwise_or_words);
}

Integer Integer::operator^(const Integer &other) const {
  return bitwise_op(*this, other, bitwise_xor_words);
}

Integer Integer::operator~() const {
  return -*this - 1;
}

Integer &Integer::operator%=(const Integer &other) {
  static_cast<BigInt *>(m_value)->operator%=(*static_cast<BigInt *>(other.m_value));
  return *this;
}

Integer &Integer::operator<<=(uint32_t other) {
  Integer result = *this << other;
  *this = std::move(result);
  return *this;
}

Integer &Integer::operator>>=(uint32_t other) {
  Integer result = *this >> other;
  *this = std::move(result);
  return *this;
}

Integer &Integer::operator&=(const Integer &other) {
  Integer result = *this & other;
  *this = std::move(result);
  return *this;
}

Integer &Integer::operator|=(const Integer &other) {
  Integer result = *this | other;
  *this = std::move(result);
  return *this;
}

Integer &Integer::operator^=(const Integer &other) {
  Integer result = *this ^ other;
  *this = std::move(result);
  return *this;
}

Integer &Integer::operator+=(const Integer &other) {
  Integer result = *this + other;
  *this = std::move(result);
  return *this;
}

Integer &Integer::operator-=(const Integer &other) {
  Integer result = *this - other;
  *this = std::move(result);
  return *this;
}

Integer &Integer::operator*=(const Integer &other) {
  Integer result = *this * other;
  *this = std::move(result);
  return *this;
}

Integer &Integer::operator/=(const Integer &other) {
  Integer result = *this / other;
  *this = std::move(result);
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
  return static_cast<const BigInt *>(m_value)->operator==(*static_cast<const BigInt *>(other.m_value
  ));
}
bool Integer::operator!=(const Integer &other) const {
  return !(*this == other);
}
bool Integer::operator<(const Integer &other) const {
  return static_cast<const BigInt *>(m_value)->operator<(*static_cast<const BigInt *>(other.m_value)
  );
}
bool Integer::operator<=(const Integer &other) const {
  return static_cast<const BigInt *>(m_value)->operator<=(*static_cast<const BigInt *>(other.m_value
  ));
}
bool Integer::operator>(const Integer &other) const {
  return !(*this <= other);
}
bool Integer::operator>=(const Integer &other) const {
  return !(*this < other);
}

Integer clear_low_64_bits(const Integer &value) {
  Integer rshift = (value >> 64);
  if (rshift == -1) {
    return 0;
  }
  return rshift << 64;
}

Integer shift_right_n_words(const Integer &value, uint32_t n) {
  auto result = value >> (n * 64);
  if (result == -1) {
    return 0;
  }
  return result;
}

uint64_t Integer::extract_low_bits() {
  Integer r = *this % TWO_64;
  bool adjusted = false;
  if (r < 0) {
    r += TWO_64;
    adjusted = true;
  }
  *this /= TWO_64;
  if (adjusted) {
    --*this;
  }
  return r.to_uint64();
}

Integer Integer::bitwise_op(
    const Integer &a, const Integer &b, uint64_t (*op)(uint64_t, uint64_t)
) {
  List<uint64_t> a_words;
  List<uint64_t> b_words;
  Integer a_clone(a);
  Integer b_clone(b);
  while (true) {
    uint64_t a_word = a_clone.extract_low_bits();
    a_words.push_back(a_word);
    uint64_t b_word = b_clone.extract_low_bits();
    b_words.push_back(b_word);

    bool a_is_extraced = (a_clone == 0 && !highest_bit_is_set(a_word)) ||
                         (a_clone == -1 && highest_bit_is_set(a_word));
    bool b_is_extraced = (b_clone == 0 && !highest_bit_is_set(b_word)) ||
                         (b_clone == -1 && highest_bit_is_set(b_word));

    if (a_is_extraced && b_is_extraced) {
      break;
    }
  }

  Integer result;
  Integer multiplier(1);
  uint64_t last_word;
  for (size_t i = 0; i < a_words.size(); ++i) {
    last_word = op(a_words[i], b_words[i]);
    result += Integer(last_word) * multiplier;
    multiplier *= TWO_64;
  }
  if (highest_bit_is_set(last_word)) {
    result -= multiplier;
  }
  return result;
}

} // namespace amelia
