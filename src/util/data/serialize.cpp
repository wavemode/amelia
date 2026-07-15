#include "serialize.hpp"

#include "util/data/pair.hpp"
#include "util/data/string.hpp"
#include "util/data/text_utils.hpp"

namespace amelia {

namespace {
struct SerializeObject {
  String name;
  List<Pair<String, Serialize>> fields;
};

struct SerializeList {
  List<Serialize> items;
};

struct SerializeString {
  String value;
};

struct SerializeBoolean {
  bool value;
};

struct SerializeInteger {
  int64_t value;
};

struct SerializeDouble {
  double value;
};

} // namespace

namespace internal {

class SerializeState {
public:
  SerializeState(AbstractString &out) : m_out(out), m_items_printed(0), m_indent_level(0) {}

  void serialize(const Serialize &obj) {
    switch (obj.m_kind) {
    case SerializeKind::Null:
      break;
    case SerializeKind::Object:
      serialize_object(*static_cast<const SerializeObject *>(obj.m_data));
      break;
    case SerializeKind::List:
      serialize_tuple(*static_cast<const SerializeList *>(obj.m_data));
      break;
    case SerializeKind::String:
      serialize_string(*static_cast<const SerializeString *>(obj.m_data));
      break;
    case SerializeKind::Integer:
      serialize_integer(*static_cast<const SerializeInteger *>(obj.m_data));
      break;
    case SerializeKind::Boolean:
      serialize_boolean(*static_cast<const SerializeBoolean *>(obj.m_data));
      break;
    case SerializeKind::Double:
      serialize_double(*static_cast<const SerializeDouble *>(obj.m_data));
      break;
    }
  }

  void serialize_object(const SerializeObject &detail) {
    uint32_t old_items_printed = m_items_printed;
    m_items_printed = 0;

    if (detail.name.size() > 0) {
      m_out.append(detail.name);
      m_out.append('(');
    } else {
      m_out.append('{');
    }

    m_indent_level += 2;
    for (const auto &field : detail.fields) {
      open_line();
      m_out.append(field.first);
      m_out.append("=");
      serialize(field.second);
      ++m_items_printed;
    }
    m_indent_level -= 2;

    if (m_items_printed > 0) {
      open_line(false);
    }

    if (detail.name.size() > 0) {
      m_out.append(')');
    } else {
      m_out.append('}');
    }

    m_items_printed = old_items_printed;
  }

  void serialize_tuple(const SerializeList &detail) {
    uint32_t old_items_printed = m_items_printed;
    m_items_printed = 0;

    m_out.append('[');

    m_indent_level += 2;
    for (const Serialize &item : detail.items) {
      open_line();
      serialize(item);
      ++m_items_printed;
    }
    m_indent_level -= 2;

    if (m_items_printed > 0) {
      open_line(false);
    }

    m_out.append(']');

    m_items_printed = old_items_printed;
  }

  void serialize_string(const SerializeString &detail) {
    m_out.append(detail.value);
  }

  void serialize_integer(const SerializeInteger &detail) {
    TextUtils::to_string(m_out, detail.value);
  }

  void serialize_boolean(const SerializeBoolean &detail) {
    TextUtils::to_string(m_out, detail.value);
  }

  void serialize_double(const SerializeDouble &detail) {
    TextUtils::to_string(m_out, detail.value);
  }

private:
  void open_line(bool with_comma = true) {
    if (with_comma && m_items_printed > 0) {
      m_out.append(',');
    }
    m_out.append('\n');
    print_indent();
  }

  void print_indent() {
    for (uint32_t i = 0; i < m_indent_level; ++i) {
      m_out.append(' ');
    }
  }

  AbstractString &m_out;
  uint32_t m_items_printed;
  uint32_t m_indent_level;
};

} // namespace internal

Serialize::Serialize() : m_kind(SerializeKind::Null), m_data(nullptr) {}

Serialize::Serialize(const Serialize &other) : m_kind(other.m_kind), m_data(other.clone()) {}

Serialize &Serialize::operator=(const Serialize &other) {
  if (this != &other) {
    clear();
    m_kind = other.m_kind;
    m_data = other.clone();
  }
  return *this;
}

Serialize::Serialize(Serialize &&other) noexcept : m_kind(other.m_kind), m_data(other.m_data) {
  other.m_kind = SerializeKind::Null;
  other.m_data = nullptr;
}

Serialize &Serialize::operator=(Serialize &&other) noexcept {
  if (this != &other) {
    clear();
    m_kind = other.m_kind;
    m_data = other.m_data;
    other.m_kind = SerializeKind::Null;
    other.m_data = nullptr;
  }
  return *this;
}

Serialize::~Serialize() {
  clear();
}

Serialize Serialize::literal(String value) {
  Serialize result;
  result.m_data = new SerializeString{.value = value};
  result.m_kind = SerializeKind::String;
  return result;
}

Serialize Serialize::quoted(String value) {
  Serialize result;
  auto data = new SerializeString{};
  data->value.append('"');
  data->value.append(value);
  data->value.append('"');
  result.m_data = data;
  result.m_kind = SerializeKind::String;
  return result;
}

Serialize Serialize::of(bool value) {
  Serialize result;
  result.m_data = new SerializeBoolean{.value = value};
  result.m_kind = SerializeKind::Boolean;
  return result;
}

Serialize Serialize::of(int64_t value) {
  Serialize result;
  result.m_data = new SerializeInteger{.value = value};
  result.m_kind = SerializeKind::Integer;
  return result;
}

Serialize Serialize::of(double value) {
  Serialize result;
  result.m_data = new SerializeDouble{.value = value};
  result.m_kind = SerializeKind::Double;
  return result;
}

void Serialize::set_object_name(String name) {
  if (m_kind == SerializeKind::Null) {
    m_data = new SerializeObject();
    m_kind = SerializeKind::Object;
  }
  if (m_kind != SerializeKind::Object) {
    throw RuntimeError("Cannot set object name on a non-object Serialize");
  }
  static_cast<SerializeObject *>(m_data)->name = move(name);
}

void Serialize::add_object_field(String field_name, Serialize field_value) {
  if (m_kind == SerializeKind::Null) {
    m_data = new SerializeObject();
    m_kind = SerializeKind::Object;
  }
  if (m_kind != SerializeKind::Object) {
    throw RuntimeError("Cannot add object field to a non-object Serialize");
  }
  static_cast<SerializeObject *>(m_data)->fields.push_back({move(field_name), move(field_value)});
}

void Serialize::add_list_item(Serialize item) {
  if (m_kind == SerializeKind::Null) {
    m_data = new SerializeList();
    m_kind = SerializeKind::List;
  }
  if (m_kind != SerializeKind::List) {
    throw RuntimeError("Cannot add tuple item to a non-tuple Serialize");
  }
  static_cast<SerializeList *>(m_data)->items.push_back(move(item));
}

void Serialize::to_string(AbstractString &out) const {
  internal::SerializeState(out).serialize(*this);
}

Serialize Serialize::quoted() const {
  String repr;
  repr.append('"');
  internal::SerializeState(repr).serialize(*this);
  repr.append('"');
  return Serialize::literal(move(repr));
}

void Serialize::clear() {
  switch (m_kind) {
  case SerializeKind::Null:
    break;
  case SerializeKind::Object:
    delete static_cast<SerializeObject *>(m_data);
    break;
  case SerializeKind::List:
    delete static_cast<SerializeList *>(m_data);
    break;
  case SerializeKind::String:
    delete static_cast<SerializeString *>(m_data);
    break;
  case SerializeKind::Boolean:
    delete static_cast<SerializeBoolean *>(m_data);
    break;
  case SerializeKind::Integer:
    delete static_cast<SerializeInteger *>(m_data);
    break;
  case SerializeKind::Double:
    delete static_cast<SerializeDouble *>(m_data);
    break;
  }
  m_kind = SerializeKind::Null;
  m_data = nullptr;
}

void *Serialize::clone() const {
  switch (m_kind) {
  case SerializeKind::Null:
    return nullptr;
  case SerializeKind::Object:
    return new SerializeObject(*static_cast<SerializeObject *>(m_data));
  case SerializeKind::List:
    return new SerializeList(*static_cast<SerializeList *>(m_data));
  case SerializeKind::String:
    return new SerializeString(*static_cast<SerializeString *>(m_data));
  case SerializeKind::Boolean:
    return new SerializeBoolean(*static_cast<SerializeBoolean *>(m_data));
  case SerializeKind::Integer:
    return new SerializeInteger(*static_cast<SerializeInteger *>(m_data));
  case SerializeKind::Double:
    return new SerializeDouble(*static_cast<SerializeDouble *>(m_data));
  }
}

} // namespace amelia
