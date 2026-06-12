#include "pretty_print.hpp"

#include "data/util/text_utils.hpp"

namespace amelia {

namespace internal {

class PrettyPrintState {
public:
  PrettyPrintState(AbstractString &out) : m_out(out), m_items_printed(0), m_indent_level(0) {}

  void pretty_print(const PrettyPrint &obj) {
    if (obj.m_detail.is_null()) {
      return;
    }
    switch (obj.m_kind) {
    case PrettyPrintKind::Object:
      pretty_print_object(static_cast<const PrettyPrint::PrettyPrintDetailObject &>(*obj.m_detail));
      break;
    case PrettyPrintKind::List:
      pretty_print_list(static_cast<const PrettyPrint::PrettyPrintDetailList &>(*obj.m_detail));
      break;
    case PrettyPrintKind::String:
      pretty_print_string(static_cast<const PrettyPrint::PrettyPrintDetailString &>(*obj.m_detail));
      break;
    case PrettyPrintKind::Integer:
      pretty_print_integer(static_cast<const PrettyPrint::PrettyPrintDetailInteger &>(*obj.m_detail)
      );
      break;
    case PrettyPrintKind::Boolean:
      pretty_print_boolean(static_cast<const PrettyPrint::PrettyPrintDetailBoolean &>(*obj.m_detail)
      );
      break;
    case PrettyPrintKind::Double:
      pretty_print_double(static_cast<const PrettyPrint::PrettyPrintDetailDouble &>(*obj.m_detail));
      break;
    }
  }

  void pretty_print_object(const PrettyPrint::PrettyPrintDetailObject &detail) {
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

  void pretty_print_list(const PrettyPrint::PrettyPrintDetailList &detail) {
    uint32_t old_items_printed = m_items_printed;
    m_items_printed = 0;

    m_out.append('[');

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
    m_out.append(']');

    m_items_printed = old_items_printed;
  }

  void pretty_print_string(const PrettyPrint::PrettyPrintDetailString &detail) {
    m_out.append(detail.value);
  }

  void pretty_print_integer(const PrettyPrint::PrettyPrintDetailInteger &detail) {
    TextUtils::to_string(m_out, detail.value);
  }

  void pretty_print_boolean(const PrettyPrint::PrettyPrintDetailBoolean &detail) {
    TextUtils::to_string(m_out, detail.value);
  }

  void pretty_print_double(const PrettyPrint::PrettyPrintDetailDouble &detail) {
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

PrettyPrint::PrettyPrint(PrettyPrintObject &&other) : PrettyPrint(move(other.m_pretty_print)) {}
PrettyPrint::PrettyPrint(PrettyPrintList &&other) : PrettyPrint(move(other.m_pretty_print)) {}
PrettyPrint::PrettyPrint(PrettyPrintString &&other) : PrettyPrint(move(other.m_pretty_print)) {}
PrettyPrint::PrettyPrint(PrettyPrintInteger &&other) : PrettyPrint(move(other.m_pretty_print)) {}
PrettyPrint::PrettyPrint(PrettyPrintBoolean &&other) : PrettyPrint(move(other.m_pretty_print)) {}
PrettyPrint::PrettyPrint(PrettyPrintDouble &&other) : PrettyPrint(move(other.m_pretty_print)) {}

PrettyPrint::PrettyPrint(const PrettyPrint &other) {
  m_kind = other.m_kind;
  switch (m_kind) {
  case PrettyPrintKind::Object:
    m_detail = Box(static_cast<const PrettyPrintDetailObject &>(*other.m_detail));
    break;
  case PrettyPrintKind::List:
    m_detail = Box(static_cast<const PrettyPrintDetailList &>(*other.m_detail));
    break;
  case PrettyPrintKind::String:
    m_detail = Box(static_cast<const PrettyPrintDetailString &>(*other.m_detail));
    break;
  case PrettyPrintKind::Integer:
    m_detail = Box(static_cast<const PrettyPrintDetailInteger &>(*other.m_detail));
    break;
  case PrettyPrintKind::Boolean:
    m_detail = Box(static_cast<const PrettyPrintDetailBoolean &>(*other.m_detail));
    break;
  case PrettyPrintKind::Double:
    m_detail = Box(static_cast<const PrettyPrintDetailDouble &>(*other.m_detail));
    break;
  }
}

PrettyPrint::PrettyPrint(PrettyPrint &&other) noexcept
    : m_kind(other.m_kind), m_detail(move(other.m_detail)) {}

PrettyPrint &PrettyPrint::operator=(const PrettyPrint &other) {
  if (this != &other) {
    m_kind = other.m_kind;
    switch (m_kind) {
    case PrettyPrintKind::Object:
      m_detail = Box(static_cast<const PrettyPrintDetailObject &>(*other.m_detail));
      break;
    case PrettyPrintKind::List:
      m_detail = Box(static_cast<const PrettyPrintDetailList &>(*other.m_detail));
      break;
    case PrettyPrintKind::String:
      m_detail = Box(static_cast<const PrettyPrintDetailString &>(*other.m_detail));
      break;
    case PrettyPrintKind::Integer:
      m_detail = Box(static_cast<const PrettyPrintDetailInteger &>(*other.m_detail));
      break;
    case PrettyPrintKind::Boolean:
      m_detail = Box(static_cast<const PrettyPrintDetailBoolean &>(*other.m_detail));
      break;
    case PrettyPrintKind::Double:
      m_detail = Box(static_cast<const PrettyPrintDetailDouble &>(*other.m_detail));
      break;
    }
  }
  return *this;
}

PrettyPrint &PrettyPrint::operator=(PrettyPrint &&other) noexcept {
  if (this != &other) {
    m_kind = other.m_kind;
    m_detail = move(other.m_detail);
  }
  return *this;
}

PrettyPrint::~PrettyPrint() = default;

PrettyPrint PrettyPrint::of(String value) {
  return PrettyPrintString(move(value));
}

PrettyPrint PrettyPrint::of(int64_t value) {
  return PrettyPrintInteger(value);
}

PrettyPrint PrettyPrint::of(bool value) {
  return PrettyPrintBoolean(value);
}

PrettyPrint PrettyPrint::of(double value) {
  return PrettyPrintDouble(value);
}

void PrettyPrint::to_string(AbstractString &out) const {
  internal::PrettyPrintState state(out);
  state.pretty_print(*this);
}

} // namespace amelia
