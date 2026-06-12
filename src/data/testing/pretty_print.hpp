#pragma once

#include "prelude.hpp"

namespace amelia {

namespace internal {
class PrettyPrintState;
}

enum class PrettyPrintKind : unsigned char {
  Object,
  List,
  String,
  Integer,
  Boolean,
  Double,
};

class PrettyPrintObject;
class PrettyPrintList;
class PrettyPrintString;
class PrettyPrintInteger;
class PrettyPrintBoolean;
class PrettyPrintDouble;

class PrettyPrint {
public:
  PrettyPrint(PrettyPrintObject &&);
  PrettyPrint(PrettyPrintList &&);
  PrettyPrint(PrettyPrintString &&);
  PrettyPrint(PrettyPrintInteger &&);
  PrettyPrint(PrettyPrintBoolean &&);
  PrettyPrint(PrettyPrintDouble &&);

  PrettyPrint(const PrettyPrint &);
  PrettyPrint &operator=(const PrettyPrint &);
  PrettyPrint(PrettyPrint &&) noexcept;
  PrettyPrint &operator=(PrettyPrint &&) noexcept;
  virtual ~PrettyPrint();

  static PrettyPrint of(String value);
  static PrettyPrint of(int64_t value);
  static PrettyPrint of(bool value);
  static PrettyPrint of(double value);
  template <typename T> static PrettyPrint of(T value) {
    return value.pretty_print();
  }

  void to_string(AbstractString &out) const;

  friend class internal::PrettyPrintState;
  friend class PrettyPrintObject;
  friend class PrettyPrintList;
  friend class PrettyPrintString;
  friend class PrettyPrintInteger;
  friend class PrettyPrintBoolean;
  friend class PrettyPrintDouble;

private:
  struct PrettyPrintDetailBase {
    virtual ~PrettyPrintDetailBase() = default;
  };

  struct PrettyPrintDetailList : PrettyPrintDetailBase {
    List<PrettyPrint> items;
  };

  struct PrettyPrintDetailObject : PrettyPrintDetailBase {
    String name;
    List<Pair<String, PrettyPrint>> fields;
  };

  struct PrettyPrintDetailString : PrettyPrintDetailBase {
    String value;
  };

  struct PrettyPrintDetailInteger : PrettyPrintDetailBase {
    int64_t value;
  };

  struct PrettyPrintDetailBoolean : PrettyPrintDetailBase {
    bool value;
  };

  struct PrettyPrintDetailDouble : PrettyPrintDetailBase {
    double value;
  };

  PrettyPrint(PrettyPrintKind kind, Box<PrettyPrintDetailBase> detail)
      : m_kind(kind), m_detail(move(detail)) {}

  PrettyPrintKind m_kind;
  Box<PrettyPrintDetailBase> m_detail;
};

class PrettyPrintList {
public:
  PrettyPrintList()
      : m_pretty_print(PrettyPrintKind::List, Box(PrettyPrint::PrettyPrintDetailList())) {}

  void add_item(PrettyPrint item) {
    PrettyPrint::PrettyPrintDetailList &detail = static_cast<PrettyPrint::PrettyPrintDetailList &>(
        *m_pretty_print.m_detail
    );
    detail.items.push_back(move(item));
  }

  friend class PrettyPrint;

private:
  PrettyPrint m_pretty_print;
};
class PrettyPrintObject {
public:
  PrettyPrintObject()
      : m_pretty_print(PrettyPrintKind::Object, Box(PrettyPrint::PrettyPrintDetailObject())) {}

  void set_name(String name) {
    PrettyPrint::PrettyPrintDetailObject
        &detail = static_cast<PrettyPrint::PrettyPrintDetailObject &>(*m_pretty_print.m_detail);
    detail.name = move(name);
  }

  void add_field(String name, PrettyPrint value) {
    PrettyPrint::PrettyPrintDetailObject
        &detail = static_cast<PrettyPrint::PrettyPrintDetailObject &>(*m_pretty_print.m_detail);
    detail.fields.push_back({move(name), move(value)});
  }

  friend class PrettyPrint;

private:
  PrettyPrint m_pretty_print;
};

class PrettyPrintString {
public:
  PrettyPrintString(String value)
      : m_pretty_print(PrettyPrintKind::String, Box(PrettyPrint::PrettyPrintDetailString())) {
    PrettyPrint::PrettyPrintDetailString
        &detail = static_cast<PrettyPrint::PrettyPrintDetailString &>(*m_pretty_print.m_detail);
    detail.value = move(value);
  }

  static PrettyPrintString quoted(String value) {
    String quoted_value = "\"";
    quoted_value.append(value);
    quoted_value.append("\"");
    return PrettyPrintString(move(quoted_value));
  }

  friend class PrettyPrint;

private:
  PrettyPrint m_pretty_print;
};

class PrettyPrintInteger {
public:
  PrettyPrintInteger(int64_t value)
      : m_pretty_print(PrettyPrintKind::Integer, Box(PrettyPrint::PrettyPrintDetailInteger())) {
    PrettyPrint::PrettyPrintDetailInteger
        &detail = static_cast<PrettyPrint::PrettyPrintDetailInteger &>(*m_pretty_print.m_detail);
    detail.value = value;
  }

  friend class PrettyPrint;

private:
  PrettyPrint m_pretty_print;
};

class PrettyPrintBoolean {
public:
  PrettyPrintBoolean(bool value)
      : m_pretty_print(PrettyPrintKind::Boolean, Box(PrettyPrint::PrettyPrintDetailBoolean())) {
    PrettyPrint::PrettyPrintDetailBoolean
        &detail = static_cast<PrettyPrint::PrettyPrintDetailBoolean &>(*m_pretty_print.m_detail);
    detail.value = value;
  }

  friend class PrettyPrint;

private:
  PrettyPrint m_pretty_print;
};

class PrettyPrintDouble {
public:
  PrettyPrintDouble(double value)
      : m_pretty_print(PrettyPrintKind::Double, Box(PrettyPrint::PrettyPrintDetailDouble())) {
    PrettyPrint::PrettyPrintDetailDouble
        &detail = static_cast<PrettyPrint::PrettyPrintDetailDouble &>(*m_pretty_print.m_detail);
    detail.value = value;
  }

  friend class PrettyPrint;

private:
  PrettyPrint m_pretty_print;
};

} // namespace amelia
