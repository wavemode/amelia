#pragma once

#include <cstdint>

namespace amelia {

namespace internal {
class PrettyPrintState;
}

struct AbstractString;
class String;

enum class PrettyPrintKind : unsigned char {
  Null,
  Object,
  Tuple,
  String,
  Boolean,
  Integer,
  Double,
};

class PrettyPrint {
public:
  PrettyPrint();
  PrettyPrint(const PrettyPrint &);
  PrettyPrint &operator=(const PrettyPrint &);
  PrettyPrint(PrettyPrint &&) noexcept;
  PrettyPrint &operator=(PrettyPrint &&) noexcept;
  ~PrettyPrint();

  static PrettyPrint literal(String value);
  static PrettyPrint quoted(String value);
  static PrettyPrint of(bool value);
  static PrettyPrint of(int64_t value);
  static PrettyPrint of(double value);
  template <typename T> static PrettyPrint of(T value) {
    return value.pretty_print();
  }

  void set_object_name(String name);
  void add_object_field(String field_name, PrettyPrint field_value);
  void set_tuple_name(String name);
  void add_tuple_item(PrettyPrint item);

  void to_string(AbstractString &out) const;

  friend class internal::PrettyPrintState;

private:
  void clear();
  void *clone() const;

  PrettyPrintKind m_kind;
  void *m_data;
};

} // namespace amelia
