#include "pretty_print.hpp"

#include "prelude.hpp"

#include "data/util/text_utils.hpp"

namespace amelia {

namespace {
struct PrettyPrintObject {
  String name;
  List<Pair<String, PrettyPrint>> fields;
};

struct PrettyPrintTuple {
  String name;
  List<PrettyPrint> items;
};

struct PrettyPrintString {
  String value;
};

struct PrettyPrintBoolean {
  bool value;
};

struct PrettyPrintInteger {
  int64_t value;
};

struct PrettyPrintDouble {
  double value;
};

} // namespace

namespace internal {

class PrettyPrintState {
public:
  PrettyPrintState(AbstractString &out) : m_out(out), m_items_printed(0), m_indent_level(0) {}

  void pretty_print(const PrettyPrint &obj) {
    switch (obj.m_kind) {
    case PrettyPrintKind::Null:
      pretty_print_null();
      break;
    case PrettyPrintKind::Object:
      pretty_print_object(*static_cast<const PrettyPrintObject *>(obj.m_data));
      break;
    case PrettyPrintKind::Tuple:
      pretty_print_tuple(*static_cast<const PrettyPrintTuple *>(obj.m_data));
      break;
    case PrettyPrintKind::String:
      pretty_print_string(*static_cast<const PrettyPrintString *>(obj.m_data));
      break;
    case PrettyPrintKind::Integer:
      pretty_print_integer(*static_cast<const PrettyPrintInteger *>(obj.m_data));
      break;
    case PrettyPrintKind::Boolean:
      pretty_print_boolean(*static_cast<const PrettyPrintBoolean *>(obj.m_data));
      break;
    case PrettyPrintKind::Double:
      pretty_print_double(*static_cast<const PrettyPrintDouble *>(obj.m_data));
      break;
    }
  }

  void pretty_print_object(const PrettyPrintObject &detail) {
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
      pretty_print(field.second);
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

  void pretty_print_tuple(const PrettyPrintTuple &detail) {
    uint32_t old_items_printed = m_items_printed;
    m_items_printed = 0;

    if (detail.name.size() == 0) {
      m_out.append('[');
    } else {
      m_out.append(detail.name);
      m_out.append('(');
    }

    m_indent_level += 2;
    for (const PrettyPrint &item : detail.items) {
      open_line();
      pretty_print(item);
      ++m_items_printed;
    }
    m_indent_level -= 2;

    if (m_items_printed > 0) {
      open_line(false);
    }

    if (detail.name.size() == 0) {
      m_out.append(']');
    } else {
      m_out.append(')');
    }

    m_items_printed = old_items_printed;
  }

  void pretty_print_null() {
    m_out.append("null");
  }

  void pretty_print_string(const PrettyPrintString &detail) {
    m_out.append(detail.value);
  }

  void pretty_print_integer(const PrettyPrintInteger &detail) {
    TextUtils::to_string(m_out, detail.value);
  }

  void pretty_print_boolean(const PrettyPrintBoolean &detail) {
    TextUtils::to_string(m_out, detail.value);
  }

  void pretty_print_double(const PrettyPrintDouble &detail) {
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

PrettyPrint::PrettyPrint() : m_kind(PrettyPrintKind::Null), m_data(nullptr) {}

PrettyPrint::PrettyPrint(const PrettyPrint &other) : m_kind(other.m_kind), m_data(other.clone()) {}

PrettyPrint &PrettyPrint::operator=(const PrettyPrint &other) {
  if (this != &other) {
    clear();
    m_kind = other.m_kind;
    m_data = other.clone();
  }
  return *this;
}

PrettyPrint::PrettyPrint(PrettyPrint &&other) noexcept
    : m_kind(other.m_kind), m_data(other.m_data) {
  other.m_kind = PrettyPrintKind::Null;
  other.m_data = nullptr;
}

PrettyPrint &PrettyPrint::operator=(PrettyPrint &&other) noexcept {
  if (this != &other) {
    clear();
    m_kind = other.m_kind;
    m_data = other.m_data;
    other.m_kind = PrettyPrintKind::Null;
    other.m_data = nullptr;
  }
  return *this;
}

PrettyPrint::~PrettyPrint() {
  clear();
}

PrettyPrint PrettyPrint::literal(String value) {
  PrettyPrint result;
  result.m_data = new PrettyPrintString{.value = value};
  result.m_kind = PrettyPrintKind::String;
  return result;
}

PrettyPrint PrettyPrint::quoted(String value) {
  PrettyPrint result;
  auto data = new PrettyPrintString{};
  data->value.append('"');
  data->value.append(value);
  data->value.append('"');
  result.m_data = data;
  result.m_kind = PrettyPrintKind::String;
  return result;
}

PrettyPrint PrettyPrint::of(bool value) {
  PrettyPrint result;
  result.m_data = new PrettyPrintBoolean{.value = value};
  result.m_kind = PrettyPrintKind::Boolean;
  return result;
}

PrettyPrint PrettyPrint::of(int64_t value) {
  PrettyPrint result;
  result.m_data = new PrettyPrintInteger{.value = value};
  result.m_kind = PrettyPrintKind::Integer;
  return result;
}

PrettyPrint PrettyPrint::of(double value) {
  PrettyPrint result;
  result.m_data = new PrettyPrintDouble{.value = value};
  result.m_kind = PrettyPrintKind::Double;
  return result;
}

void PrettyPrint::set_object_name(String name) {
  if (m_kind == PrettyPrintKind::Null) {
    m_data = new PrettyPrintObject();
    m_kind = PrettyPrintKind::Object;
  }
  if (m_kind != PrettyPrintKind::Object) {
    throw RuntimeError("Cannot set object name on a non-object PrettyPrint");
  }
  static_cast<PrettyPrintObject *>(m_data)->name = move(name);
}

void PrettyPrint::add_object_field(String field_name, PrettyPrint field_value) {
  if (m_kind == PrettyPrintKind::Null) {
    m_data = new PrettyPrintObject();
    m_kind = PrettyPrintKind::Object;
  }
  if (m_kind != PrettyPrintKind::Object) {
    throw RuntimeError("Cannot add object field to a non-object PrettyPrint");
  }
  static_cast<PrettyPrintObject *>(m_data)->fields.push_back({move(field_name), move(field_value)});
}

void PrettyPrint::set_tuple_name(String name) {
  if (m_kind == PrettyPrintKind::Null) {
    m_data = new PrettyPrintTuple();
    m_kind = PrettyPrintKind::Tuple;
  }
  if (m_kind != PrettyPrintKind::Tuple) {
    throw RuntimeError("Cannot set tuple name on a non-tuple PrettyPrint");
  }
  static_cast<PrettyPrintTuple *>(m_data)->name = move(name);
}

void PrettyPrint::add_tuple_item(PrettyPrint item) {
  if (m_kind == PrettyPrintKind::Null) {
    m_data = new PrettyPrintTuple();
    m_kind = PrettyPrintKind::Tuple;
  }
  if (m_kind != PrettyPrintKind::Tuple) {
    throw RuntimeError("Cannot add tuple item to a non-tuple PrettyPrint");
  }
  static_cast<PrettyPrintTuple *>(m_data)->items.push_back(move(item));
}

void PrettyPrint::to_string(AbstractString &out) const {
  internal::PrettyPrintState(out).pretty_print(*this);
}

void PrettyPrint::clear() {
  switch (m_kind) {
  case PrettyPrintKind::Null:
    break;
  case PrettyPrintKind::Object:
    delete static_cast<PrettyPrintObject *>(m_data);
    break;
  case PrettyPrintKind::Tuple:
    delete static_cast<PrettyPrintTuple *>(m_data);
    break;
  case PrettyPrintKind::String:
    delete static_cast<PrettyPrintString *>(m_data);
    break;
  case PrettyPrintKind::Boolean:
    delete static_cast<PrettyPrintBoolean *>(m_data);
    break;
  case PrettyPrintKind::Integer:
    delete static_cast<PrettyPrintInteger *>(m_data);
    break;
  case PrettyPrintKind::Double:
    delete static_cast<PrettyPrintDouble *>(m_data);
    break;
  }
  m_kind = PrettyPrintKind::Null;
  m_data = nullptr;
}

void *PrettyPrint::clone() const {
  switch (m_kind) {
  case PrettyPrintKind::Null:
    return nullptr;
  case PrettyPrintKind::Object:
    return new PrettyPrintObject(*static_cast<PrettyPrintObject *>(m_data));
  case PrettyPrintKind::Tuple:
    return new PrettyPrintTuple(*static_cast<PrettyPrintTuple *>(m_data));
  case PrettyPrintKind::String:
    return new PrettyPrintString(*static_cast<PrettyPrintString *>(m_data));
  case PrettyPrintKind::Boolean:
    return new PrettyPrintBoolean(*static_cast<PrettyPrintBoolean *>(m_data));
  case PrettyPrintKind::Integer:
    return new PrettyPrintInteger(*static_cast<PrettyPrintInteger *>(m_data));
  case PrettyPrintKind::Double:
    return new PrettyPrintDouble(*static_cast<PrettyPrintDouble *>(m_data));
  }
}

} // namespace amelia
