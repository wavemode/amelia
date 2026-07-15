#pragma once

#include <cstdint>

namespace amelia {

namespace internal {
class SerializeState;
}

struct AbstractString;
class String;

enum class SerializeKind : uint8_t {
  Null,
  Object,
  List,
  String,
  Boolean,
  Integer,
  Double,
};

class Serialize {
public:
  Serialize();
  Serialize(const Serialize &);
  Serialize &operator=(const Serialize &);
  Serialize(Serialize &&) noexcept;
  Serialize &operator=(Serialize &&) noexcept;
  ~Serialize();

  static Serialize literal(String value);
  static Serialize quoted(String value);
  static Serialize of(bool value);
  static Serialize of(int64_t value);
  static Serialize of(double value);
  template <typename T> static Serialize of(T value) {
    return value.serialize();
  }

  void set_object_name(String name);
  void add_object_field(String field_name, Serialize field_value);
  void add_list_item(Serialize item);

  void to_string(AbstractString &out) const;
  Serialize quoted() const;

  friend class internal::SerializeState;

private:
  void clear();
  void *clone() const;

  SerializeKind m_kind;
  void *m_data;
};

} // namespace amelia
