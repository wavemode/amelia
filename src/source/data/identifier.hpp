#pragma once

#include "util/data/string.hpp"

namespace amelia {

class Identifier {
public:
  explicit Identifier(Text name);

  Text literal() const noexcept;

  void pretty_print(AbstractString &out, bool quoted = true, bool escaped = true) const;

private:
  String m_name;
};

} // namespace amelia